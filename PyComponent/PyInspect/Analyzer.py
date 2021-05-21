#!/usr/bin/python

import sys
import inspect
import astunparse
import ast
import pickle
import os
import time
from ast import Name
from os.path import abspath, sep, join
from .ModRewriter import *
from .HandleEvent import *

InitTicks = time.time()

def TIME_COST (Name):
    print ("@@@@ ", Name, " time cost: ", str (time.time() - InitTicks))

LOAD_ONDEMAND = "LOAD_ONDEMAND"
FUNCTION_DEF_PKL = "function_def.pkl"

class ClassDef ():
    def __init__(self, ClsName, Cid):
        self.Id   = Cid
        self.Name = ClsName
        self.Funcs = []

    def AddFunc (self, Func):
        self.Funcs.append (Func)

class FuncDef ():
    def __init__(self, Cls, FName, Fid, FormalParas):
        self.Cls   = Cls
        self.Id    = Fid
        self.Name  = FName
        self.Paras = FormalParas

    def View (self):
        print ("FuncDef: Id = ", self.Id, " Name = ", self.Name, " Paras = ", self.Paras)

class Analyzer ():
    def __init__(self, PyListFile, SrcDir=".", PyMap=None):
        self.AstInfo = {}
        self.LoadPlks (SrcDir, PyListFile)
        #TIME_COST ("LoadPlks")
        
        self.FuncDef   = {}
        self.ClassDef  = {}
        self.Stmt2FuncDef = {}
        self.LoadFuncDef ()
        #TIME_COST ("LoadFuncDef")
        
        self.PyMaping  = {}
        self.LoadPyMap (PyMap)
        #TIME_COST ("LoadPyMap")

    def LoadPyMap (self, PyMap):
        if PyMap == None:
            return

        if not os.path.exists(PyMap):
            return
        
        with open(PyMap, 'r', encoding='latin1') as File:
            for line in File:
                ini, src = line.split ()
                #print (ini, " -> ", src)
                self.PyMaping [ini] = src

    def GetPySrc (self, Src):
        if Src[0:2] == "./":
            Src = Src[2:]
        if len (self.PyMaping) == 0:
            return Src
        Ini = self.PyMaping.get (Src)
        if Ini == None:
            return Src
        else:
            #print ("Match: ", Src, " -> ", Ini);
            return Ini

    def GetFuncParas (self, Stmt):
        #print (ast.dump (Stmt))
        Fparas = []
        Args = Stmt.args.args
        for arg in Args:
            if Stmt.name == "__init__" and arg.arg == "self":
                continue
            Fparas.append (arg.arg)
        return Fparas

    def ParseFuncDef (self, Stmt, ClfName=None):
        Fid = len (self.FuncDef)+1
        Paras = self.GetFuncParas (Stmt)
        if ClfName is None:
            if 'self' in Paras:
                return None
            return FuncDef ("", Stmt.name, Fid, Paras)
        else:
            FullName = ClfName + "." + Stmt.name
            return FuncDef (ClfName, FullName, Fid, Paras)
   
    def ParseClsDef (self, Stmt):
        Cid = len (self.ClassDef)+1
        Cls = ClassDef (Stmt.name, Cid)
        Body = Stmt.body
        for Fdef in Body:
            if not isinstance (Fdef, FunctionDef):
                continue
            Cls.AddFunc (Fdef.name) 
            Def = self.ParseFuncDef (Fdef, Stmt.name)
            if Def is None:
                continue
            self.FuncDef[Def.Name]  = Def 
            self.Stmt2FuncDef[Fdef] = Def
        return Cls

    def InitFuncDef (self, Line2Stmt):
        for Line, Stmt in Line2Stmt.items ():
            #print ("\r\n", ast.dump (Stmt))
            Type= Stmt.__class__.__name__
            if Type == "FunctionDef":
                Def = self.ParseFuncDef (Stmt)
                if Def is None:
                    continue
                self.FuncDef[Stmt.name] = Def
            elif Type == "ClassDef":
                self.ClassDef[Stmt.name] = self.ParseClsDef (Stmt)

    def InitClsFuncSet (self):
        self.FuncDef["pyinspect"] = FuncDef ("", "pyinspect", 1, [])
        for Mod, ModAst in self.AstInfo.items ():
            if ModAst == LOAD_ONDEMAND:
                continue
            Line2Stmt = ModAst.lineno2stmt
            self.InitFuncDef (Line2Stmt)

    def GetFuncDef (self, FuncName):
        return self.FuncDef.get (FuncName)

    def GetClsDef (self, ClsName):
        return self.ClassDef.get (ClsName)

    def LoadFuncDef (self):
        if os.path.exists (FUNCTION_DEF_PKL) == True:
            with open("function_def.pkl", 'rb') as Pkl:
                self.FuncDef = pickle.load(Pkl)
                self.FuncDef["pyinspect"] = FuncDef ("", "pyinspect", 1, [])
        
        if len (self.FuncDef) == 0:
            self.InitClsFuncSet ()
        if False:
            for name, Fdef in self.FuncDef.items ():
                print ("Func: ", Fdef.Id, " ", Fdef.Name, " ", Fdef.Paras)
        print ("self.FuncDef = ", len (self.FuncDef))

    def LoadPlkOnDemand (self, PyName):
        PlkName = str(PyName).replace(sep, '#')
        PklFile = self.SrcDir + '/cachepkl/' + PlkName + '.pkl'
        with open(PklFile, 'rb') as Pkl:
            Mod = pickle.load(Pkl)
            self.AstInfo[PyName] = Mod
            self.InitFuncDef (Mod.lineno2stmt)
            return Mod
       
    def LoadPlks(self, SrcDir, PyListFile):
        self.SrcDir = os.path.abspath(SrcDir)
        with open(PyListFile) as FList:
            PyList = FList.read().splitlines()
            if '' in PyList:
                PyList.remove('')
            #print (PyList)
            for FName in PyList:
                Name = str(FName).replace(sep, '#')
                PklFile = SrcDir + '/cachepkl/' + Name + '.pkl'
                #print('load the pickled module {%s}' %PklFile)
                with open(PklFile, 'rb') as Pkl:
                    if os.path.exists (FUNCTION_DEF_PKL) == True:
                        self.AstInfo[FName] = LOAD_ONDEMAND
                    else:
                        Mod = pickle.load(Pkl)
                        self.AstInfo[FName] = Mod
        print ("Load pkls number = ", len (self.AstInfo))
        return
        
    def GetSource(self, Object):
        if isinstance(Object, str):
            if str(Object[-4:]).lower() in ('.pyc', '.pyo'):
                return abspath(Object[:-4] + '.py')
            else:
                return abspath(Object)
        if inspect.isframe(Object):
            filename = inspect.getsourcefile(Object)
            return abspath(filename) if filename is not None else None
        else:
            return None

    def IsIgnore (self, Stmt):
        IgnoreList = ["Import", "ClassDef", "ImportFrom"]
        if Stmt.__class__.__name__ in IgnoreList:
            #print ("IsIgnore ---> ", Stmt.__class__, Stmt.__class__.__name__)
            return True
        else:
            return False

    def DebugAst (self, Line2Stmt):
        for line, stmt in Line2Stmt.items ():
            print (line, " ---> ", ast.dump (stmt))
        exit (0)

    def GetMod (self, ScriptName):
        Mod = self.AstInfo.get(ScriptName)
        if Mod == LOAD_ONDEMAND:
            return self.LoadPlkOnDemand(ScriptName)
        else:
            return Mod
    
    def HandleEvent(self, ScriptName, Frame, Event, LineNo):
        ScriptName = self.GetPySrc (ScriptName)
        Mod = self.GetMod(ScriptName)
        #Mod = self.AstInfo.get(ScriptName)
        if Mod == None:
            #print (ScriptName, " -> get ast fail!!!")
            return None
        Line2Stmt = Mod.lineno2stmt

        #self.DebugAst (Line2Stmt)

        Stmt = Line2Stmt.get(LineNo)
        if Stmt == None:
            return None

        if self.IsIgnore (Stmt) == True:
            return None

        #print (ast.dump (Stmt))
        LiveObj = None
        if Event == 'call':
            LiveObj = self.__HandleCall (Frame, Event, Stmt)
            LiveObj.SetLineNo (LineNo)
        elif Event == 'line':
            LiveObj = self.__HandleLine (Frame, Event, Stmt)
            LiveObj.SetLineNo (LineNo)
            if LiveObj.Callee != None and self.GetClsDef (LiveObj.Callee) != None:
                LiveObj.Class   = LiveObj.Callee
                LiveObj.Callee += ".__init__"
        elif Event == 'return':
            LiveObj = self.__HandleReturn (Frame, Event, Stmt)
            LiveObj.SetLineNo (LineNo)
        elif Event == 'except':
            print ("-> Except")

        return LiveObj
        
    def __HandleCall(self, Frame, ClEvent, Statement):
        CE = CallEvent (Frame, ClEvent, Statement)
        CE.GetDefUse ()
        return CE.LiveObj

    def __HandleLine(self, Frame, LnEvent, Statement):                
        LE = LineEvent (Frame, LnEvent, Statement)
        LE.GetDefUse ()
        return LE.LiveObj

    def __HandleReturn(self, Frame, RtEvent, Statement):
        RE = RetEvent (Frame, RtEvent, Statement)
        RE.GetDefUse ()
        return RE.LiveObj
        
    def __HandleExcept(self, Frame, ExpEvent, Statement):
        pass    



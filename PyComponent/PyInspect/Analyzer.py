#!/usr/bin/python

import sys
import inspect
import astunparse
import ast
import pickle
from ast import Name
from os.path import abspath, sep, join
from .ModRewriter import *
from .HandleEvent import *

class ClassDef ():
    def __init__(self, ClsName, Cid):
        self.Id   = Cid
        self.Name = ClsName
        self.Funcs = []

    def AddFunc (self, Func):
        self.Funcs.append (Func)

class FuncDef ():
    def __init__(self, FName, Fid, FormalParas):
        self.Id    = Fid
        self.Name  = FName
        self.Paras = FormalParas

    def View (self):
        print ("FuncDef: Id = ", self.Id, " Name = ", self.Name, " Paras = ", self.Paras)

class Analyzer ():
    def __init__(self, PyListFile, SrcDir="."):
        self.AstInfo   = self.LoadPlks (SrcDir, PyListFile)
        self.FuncDef   = {}
        self.ClassDef  = {}
        self.InitClsFuncSet ()
        #print ("Load AST info:", self.AstInfo)

    def GetFuncParas (self, Stmt):
        #print (ast.dump (Stmt))
        Fparas = []
        Args = Stmt.args.args
        for arg in Args:
            Fparas.append (arg.arg)
        return Fparas

    def ParseFuncDef (self, Stmt, ClfName=None):
        Fid = len (self.FuncDef)+1
        Paras = self.GetFuncParas (Stmt)
        if ClfName == None:
            return FuncDef (Stmt.name, Fid, Paras)
        else:
            FullName = ClfName + "." + Stmt.name
            return FuncDef (FullName, Fid, Paras)
   
    def ParseClsDef (self, Stmt):
        Cid = len (self.ClassDef)+1
        Cls = ClassDef (Stmt.name, Cid)
        Body = Stmt.body
        for Fdef in Body:
            if not isinstance (Fdef, FunctionDef):
                continue
            Cls.AddFunc (Fdef.name) 
            Def = self.ParseFuncDef (Fdef, Stmt.name)
            self.FuncDef[Def.Name] = Def 
        return Cls

    def InitClsFuncSet (self):
        self.FuncDef["main"] = FuncDef ("main", 1, [])
        for Mod, ModAst in self.AstInfo.items ():
            Line2Stmt = ModAst.lineno2stmt
            for Line, Stmt in Line2Stmt.items ():
                #print ("\r\n", ast.dump (Stmt))
                Type= Stmt.__class__.__name__
                if Type == "FunctionDef":
                    self.FuncDef[Stmt.name] = self.ParseFuncDef (Stmt)
                elif Type == "ClassDef":
                    self.ClassDef[Stmt.name] = self.ParseClsDef (Stmt)

        if False:
            for name, Fdef in self.FuncDef.items ():
                print ("Func: ", Fdef.Id, " ", Fdef.Name, " ", Fdef.Paras)
            for name, Cdef in self.ClassDef.items ():
                print ("Class: ", Cdef.Id, " ", Cdef.Name, " ", Cdef.Funcs)
        return

    def GetFuncDef (self, FuncName):
        return self.FuncDef.get (FuncName)

    def GetClsDef (self, ClsName):
        return self.ClassDef.get (ClsName) 
        
    def LoadPlks(self, SrcDir, PyListFile):
        AstInfo = {}
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
                    #print (Pkl)
                    Mod = pickle.load(Pkl)
                    #Path = self.GetSource (FName)
                    AstInfo[FName] = Mod
        return AstInfo
        
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

    
    def HandleEvent(self, ScriptName, Frame, Event, LineNo):
        Mod = self.AstInfo.get(ScriptName)
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



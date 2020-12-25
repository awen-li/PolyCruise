#!/usr/bin/python

import sys
import inspect
import astunparse
import ast
import pickle
from ast import Name
from os.path import abspath, sep, join
from ModRewriter import NewASTInfo
from HandleEvent import *


class Analyzer:
    def __init__(self, RecordFile):
        self.AstInfo = self.LoadPlks (RecordFile)
        self.FuncID  = {}
        self.InitFuncSet ()

    def InitFuncSet (self):
        for Mod, ModAst in self.AstInfo.items ():
            Line2Stmt = ModAst.lineno2stmt
            for Line, Stmt in Line2Stmt.items ():
                if isinstance(Stmt, FunctionDef):
                    self.FuncID[Stmt.name] = len (self.FuncID)+1
                    #print (Stmt.name, " -> ", self.FuncID[Stmt.name])
        return

    def GetFuncId (self, FuncName):
        FuncId = self.FuncID.get (FuncName)
        if FuncId == None:
            return 0
        return FuncId            
        
    def LoadPlks(self, RecordFile):
        AstInfo = {}
        with open(RecordFile) as FList:
            PyList = FList.read().splitlines()
            if '' in PyList:
                PyList.remove('')
            print (PyList)
            for FName in PyList:
                PklFile = 'cachepkl/' + FName + '.pkl'
                print('load the pickled module {%s}' %PklFile)
                with open(PklFile, 'rb') as Pkl:
                    print (Pkl)
                    Mod = pickle.load(Pkl)
                    Path = self.GetSource (FName)
                    AstInfo[Path] = Mod
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
 
    def HandleEvent(self, Module, Frame, Event, LineNo):
        Mod = self.AstInfo.get(Module)
        Line2Stmt = Mod.lineno2stmt
        #for lineNo, Stmt in Line2Stmt.items ():
        #    print (lineNo, "->", ast.dump (Stmt))
        #exit (0)

        Stmt = Line2Stmt.get(LineNo)
        if Stmt == None:
            return None

        #print (ast.dump (Stmt), end=" ")
        if Event == 'call':
            return self.__HandleCall (Frame, Event, Stmt)            
        elif Event == 'line':
            return self.__HandleLine (Frame, Event, Stmt)
        elif Event == 'return':
            return self.__HandleReturn (Frame, Event, Stmt)
        elif Event == 'except':
            print ("-> Except")
        
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



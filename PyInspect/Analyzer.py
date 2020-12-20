#!/usr/bin/python

import sys
import inspect
import astunparse
import ast
import pickle
from ast import Name
from os.path import abspath, sep, join
from ModRewriter import NewASTInfo

class Analyzer:
    def __init__(self, RecordFile):
        self.AstInfo = self.LoadPlks (RecordFile)
        
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
 
    def HandleEvent(self, Module, Event, LineNo):
        Mod = self.AstInfo.get(Module)
        Line2Stmt = Mod.lineno2stmt

        Stmt = Line2Stmt.get(LineNo)     
        if Stmt == None:
            return

        if Event == 'call':
            print ("-> call")
            self.__HandleCall (Event)            
        elif Event == 'line':
            print ("-> Line")
            #print (Stmt.value)
        elif Event == 'return':
            print ("-> Return")
        elif Event == 'except':
            print ("-> Except")
        
    def __HandleCall(self, CallEvent):
        pass

    def __HandleLine(self, LineEvent):
        Stmt = Line2Stmt.get(LineNo)
        if Stmt == None:
            return
        print (ast.dump (Stmt), end=" ")
        Def = Stmt.targets[0]
        print ("[%d]Def:%s" %(LineNo, Def.id), end = " ")
            
        Use = Stmt.value
        if isinstance(Use, Name):
            print ("Use:%s" %Use.id)
        else:
            print ("")
        
    def __HandleReturn(self, RetEvent):
        pass
        
    def __HandleExcept(self, ExpEvent):
        pass    



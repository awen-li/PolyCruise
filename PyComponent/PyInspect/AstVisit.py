#!/usr/bin/python
# _*_ coding:utf-8 _*_

import re
import ast
from ast import *
from copy import deepcopy, copy

class FuncDef ():
    def __init__(self, Cls, FName, Fid, FormalParas):
        self.Cls   = Cls
        self.Id    = Fid
        self.Name  = FName
        self.Paras = FormalParas

    def View (self):
        print ("FuncDef: Id = ", self.Id, " Name = ", self.Name, " Paras = ", self.Paras)

class ASTWalk(NodeVisitor):
    def __init__(self, LibName):
        self.LibName = LibName
        self.SrcApiDef = {}
        self.FuncDef   = {}
        self.FId = 1
    
    def visit(self, node):
        """Visit a node."""
        if node is None:
            return
        method = 'visit_' + node.__class__.__name__.lower()
        visitor = getattr(self, method, self.generic_visit)
        return visitor(node)

    def _GetArgs (self, Stmt):
        ArgList = []
        Args = Stmt.args.args
        for arg in Args:
            if Stmt.name == "__init__" and arg.arg == "self":
                continue
            ArgList.append (arg.arg)
        return ArgList

    
    def _GetFuncDef (self, Stmt, ClfName=None):
        Fid = self.FId
        self.FId += 1
        
        ArgList = self._GetArgs (Stmt)
        if ClfName == None:
            return FuncDef ("", Stmt.name, Fid, ArgList)
        else:
            FullName = ClfName + "." + Stmt.name
            return FuncDef (ClfName, FullName, Fid, ArgList)

    # Call(func=Attribute(value=Name(id='np', ctx=Load()), attr='array', ctx=Load()), 
    #      args=[Name(id='v5613', ctx=Load())], keywords=[])
    def _ProcCall(self, Caller, Stmt):
        #print ("\tcall -->", Caller, ": ", ast.dump (Stmt))
        NameMap = {"numpy":"numpy", "np":"numpy", "torch":"torch"}
        Func = Stmt.func
        if not isinstance (Func, Attribute):
            return
        if not isinstance (Func.value, Name):
            return
        
        FuncName = NameMap.get (Func.value.id)
        if FuncName == None or self.LibName.find (FuncName) == -1:
            return
        
        if hasattr (Func, "attr") != True:
            return
            
        FuncName = Func.attr
        self.SrcApiDef[FuncName] = True     

    def visit_functiondef(self, node):
        FuncName = node.name
        self.FuncDef [FuncName] = self._GetFuncDef (node)

        if FuncName[0:5] != "test_":
            return

        Body = node.body
        for Stmt in Body:
            if isinstance (Stmt, Assign):
                if isinstance (Stmt.value, Call):
                    self._ProcCall (FuncName, Stmt.value) 
            elif isinstance (Stmt, Call):
                self._ProcCall (FuncName, Stmt.value)
            else:
                continue
        return

    def visit_classdef(self, node):
        Body = node.body
        for Fdef in Body:
            if not isinstance (Fdef, FunctionDef):
                continue
            
            Def = self._GetFuncDef (Fdef, node.name)
            self.FuncDef[Def.Name]  = Def
            
            self.visit_functiondef (Fdef)
        return

    
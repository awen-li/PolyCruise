#!/usr/bin/python
# _*_ coding:utf-8 _*_

import re
import ast
from ast import *
from copy import deepcopy, copy

class ASTWalk(NodeVisitor):
    def __init__(self):
        self.FuncDef = {}
    
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

    def _ProcCall(self, Caller, Stmt):
        print ("\tcall -->", Caller, ": ", ast.dump (Stmt))

    def visit_functiondef(self, node):
        FuncName = node.name
        if FuncName[0:5] != "test_":
            return
        ArgList  = self._GetArgs (node)

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


    
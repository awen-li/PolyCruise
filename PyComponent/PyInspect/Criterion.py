#!/usr/bin/python

import sys

class CrnF ():
    def __init__(self, Name, Return, Local):
        self.Name       = Name
        self.Return     = Return
        self.Local      = Local

class Criterion ():
    def __init__(self):
        self.Criterions = {}

    def Insert (self, Function,  Return, Local):
        self.Criterions[Function] = CrnF (Function, Return, Local) 

    def GetTaintParas (self, FuncName):
        crnF = self.Criterions.get (FuncName)
        if crnF == None:
            return None
        else:
            return crnF.Parameters

    def IsCriterion (self, FuncName, Def=None):
        crnF = self.Criterions.get (FuncName)
        if crnF == None:
            return False
        else:
            if crnF.Return != 'False':
                return True
            elif Def != None and crnF.Local == Def:
                return True
            else:
                return False

        
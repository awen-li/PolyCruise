#!/usr/bin/python

import sys

class CrnF ():
    def __init__(self, Name, Return, Args):
        self.Name    = Name
        self.Return  = Return
        self.Args    = Args

class Criterion ():
    def __init__(self):
        self.Criterions = {}

    def Insert (self, Function,  Return, Args):
        self.Criterions[Function] = CrnF (Function, Return, Args)

    def GetSrcArgs (self, FuncName):
        crnF = self.Criterions.get (FuncName)
        if crnF == None or crnF.Args == "None":
            return None
        else:
            return crnF.Args

    def IsCriterion (self, FuncName):
        crnF = self.Criterions.get (FuncName)
        if crnF == None:
            return False
        else:
            if crnF.Return != 'False':
                return True
            else:
                return False

        
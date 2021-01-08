#!/usr/bin/python

import sys

class Criterion ():
    def __init__(self):
        self.Criterions = {}
        self.InitCriterion ()

    def InitCriterion (self, TaineBits=[]):
        self.Criterions["Source"] = TaineBits
        self.Criterions["DemoTr"] = [0]
    
    def Insert (self, Function, TaineBits=[]):
        self.Criterions[Function] = TaineBits 

    def GetTaintBits (self, FuncName):
        TaineBits = self.Criterions.get (FuncName)
        if TaineBits == None:
            return None
        else:
            return TaineBits

    def IsCriterion (self, FuncName):
        TaintBits = self.GetTaintBits (FuncName)
        if TaintBits == None:
            return False
        else:
            return True
        
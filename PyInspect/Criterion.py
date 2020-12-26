#!/usr/bin/python

import sys

class Criterion ():
    def __init__(self):
        self.Criterions = {}
        self.InitCriterion ()

    def InitCriterion (self):
        self.Criterions["Source"] = True

    def IsCriterion (self, FuncName):
        Exist = self.Criterions.get (FuncName)
        if Exist == None:
            return False
        else:
            return True
        
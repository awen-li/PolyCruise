#!/usr/bin/python

import sys

class Criterion ():
    def __init__(self):
        self.Criterions = {}

    def InitCriterion (self):
        self.Criterions["AddbyC"] = True

    def IsCriterion (self, FuncName):
        Exist = self.Criterion.find (FuncName)
        if Exist == None:
            return False
        else:
            return True
        
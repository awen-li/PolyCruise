#!/usr/bin/python
from PyDemo import *

class Demo ():
    def __init__ (self, value):
        self.v = value
    
def Trace (val):
    D = Demo (val)
    PwdInfo (D.v)
    Tag = "show"
    New = Tag + "->" + str(D.v)
    print ("Infor: ", New)

if __name__ == '__main__':  
    Trace (8)


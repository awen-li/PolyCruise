#!/usr/bin/python
from PyDemo import *

class Demo ():
    def __init__ (self, value):
        self.v = value
    
def DemoTr ():
    pwd = PwdInfo ()
    D = Demo (pwd)
    return D
    
def Trace (D):
    Tag = "show"
    New = Tag + "->" + D.v
    print ("Infor: ", New)

if __name__ == '__main__':  
    D = DemoTr()
    Trace (D)


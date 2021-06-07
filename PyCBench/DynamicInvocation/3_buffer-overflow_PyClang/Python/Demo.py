#!/usr/bin/python
import os
from PyDemo import *

class Demo ():
    def __init__ (self, value):
        self.v = value
    
    def Call (self):
        Module = "Demo.Call"
        PwdInfo (Module, self.v)
        
    def CallFunc(self, name: str):
        Call = f"{name}"
        if hasattr(self, Call) and callable(getattr(self, Call)):
            Call = getattr(self, Call)
            Call()
    
def DemoTr ():
    Var = os.getenv("CASE1")
    Var = int (Var)
    D = Demo (Var)
    return D

if __name__ == '__main__':  
    D = DemoTr()
    D.CallFunc ("Call")



#!/usr/bin/python
from PyDemo import *

class Demo ():
    def __init__ (self, value):
        self.v = value
        
    def Call1 (self):
        Tag = "Call1"
        New = Tag + "->" + self.v
        print ("Infor: ", New)
        
    def Call2 (self):
        Tag = "Call2"
        New = Tag + "->" + self.v
        print ("Infor: ", New)
        
    def CallFunc(self, name: str):
        Call = f"{name}"
        if hasattr(self, Call) and callable(getattr(self, Call)):
            Call = getattr(self, Call)
            Call()
    
def DemoTr ():
    pwd = PwdInfo ()
    D = Demo (pwd)
    return D

if __name__ == '__main__':  
    D = DemoTr()
    D.CallFunc ("Call1")



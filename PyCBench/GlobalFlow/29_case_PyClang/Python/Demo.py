#!/usr/bin/python
from PyDemo import *

class Demo ():
    def __init__ (self, value):
        self.v = value
    
def DemoTr (Value):
    pwd = PwdInfo ()
    D = Demo (pwd)
    New = Value + "->" + D.v
    print ("Infor: ", New)

if __name__ == '__main__':
    Tag = "show"
    DemoTr(Tag)


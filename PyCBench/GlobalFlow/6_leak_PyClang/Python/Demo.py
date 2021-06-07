#!/usr/bin/python
from PyDemo import *
    
def DemoTr (Value):
    pwd = PwdInfo ()
    New = Value + "->" + pwd
    print ("Infor: ", New)

if __name__ == '__main__':
    Tag = "show"
    DemoTr(Tag)


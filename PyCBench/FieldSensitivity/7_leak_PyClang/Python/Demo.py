#!/usr/bin/python
import os
from PyDemo import *
from DemoAdd import DemoAdd
    
def DemoTr (Value):
    Da = DemoAdd (1)
    Res = Da.Add (Value)
    DemoTrace ("Add", Res)

if __name__ == '__main__':
    Var = os.getenv("CASE1")
    Var = int (Var)
    DemoTr(Var)


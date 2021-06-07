#!/usr/bin/python
import os
from DemoAdd import DemoAdd
    
def DemoTr (Value):
    Var = os.getenv("CASE1")
    Var = int (Var)
    ca = CA (Var) 
    Da = DemoAdd (ca.v)
    Res = Da.Add (Value)
    return Res
    
def Trace (res):
    print ("trace end", res)

class CA ():
    def __init__ (self, a):
        self.v = a

if __name__ == '__main__':
    Temp = 8
    Result = DemoTr(Temp)
    Trace (Result)


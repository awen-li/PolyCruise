#!/usr/bin/python
import os
from DemoAdd import DemoAdd
    
def DemoTr (Value):
    Da = DemoAdd (Value)
    Var = eval (os.getenv("CASE1"))
    return Da.Add (Var)

if __name__ == '__main__':
    Temp = 8
    Result = DemoTr(Temp)
    print ("trace end", Result)


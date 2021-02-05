#!/usr/bin/python
import os
from DemoAdd import DemoAdd
    
def DemoTr (Value):
    Da = DemoAdd (Value)
    Var = os.getenv("CASE1")
    Var = int (Var)
    Res = Da.Add (Var)
    return Res


if __name__ == '__main__':
    Temp = 8
    Result = DemoTr(Temp)
    print ("trace end", Result)


#!/usr/bin/python
import os
from DemoAdd import DemoAdd
    
def DemoTr (Value):
    Da = DemoAdd (Value)
    Var = os.getenv("CASE1")
    if Var == "4":
    	return Da.Add (4)
    else:
    	return Da.Add (6)

if __name__ == '__main__':
    Temp = 8
    Result = DemoTr(Temp)
    print ("trace end", Result)


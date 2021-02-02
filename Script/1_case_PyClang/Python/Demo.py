#!/usr/bin/python

from PyDemo import *
from DemoAdd import DemoAdd
    
def DemoTr (Value):
    Da = DemoAdd (1)
    Res = Da.Add (Value)
    DemoTrace ("Add", Res)
    print ("trace end", Res)
    
def Source ():
	return 8

if __name__ == '__main__':
    Var = Source ()
    DemoTr(Var)


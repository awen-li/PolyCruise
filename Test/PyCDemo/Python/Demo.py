#!/usr/bin/python

import sys
import inspect
import threading
from PyDemo import *
from PyTrace import *

def Add(a, b):
    Result = a + b
    return Result
    
def DemoTr (Value):
    Res = Add (1, Value)
    DemoTrace ("Add", Res)
    print ("trace end", Res)

if __name__ == '__main__':
    PyTraceInit ()
    DemoTr(8)


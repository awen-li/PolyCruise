import builtins
import sys
import inspect
import threading
from PyDemo import *
from PyTrace import *
def Add(a, b):
    v1 = (a + b)
    Result = v1
    return Result
def DemoTr(Value):
    v2 = 1
    v3 = Add(v2, Value)
    Res = v3
    v4 = 'Add'
    DemoTrace(v4, Res)
    v5 = 'trace end'
    print(v5, Res)
v7 = '__main__'
v6 = (__name__ == v7)
if v6:
    PyTraceInit()
    v8 = 8
    DemoTr(v8)

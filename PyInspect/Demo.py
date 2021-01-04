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
v6 = '__main__'
v5 = (__name__ == v6)
if v5:
    PyTraceInit()
    v7 = 8
    DemoTr(v7)

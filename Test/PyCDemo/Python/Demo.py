#!/usr/bin/python

import sys
import inspect
import threading
from PyDemo import *
from PyTrace import *
from DemoAdd import DemoAdd
    
def DemoTr (Value):
    Da = DemoAdd (1)
    Res = Da.Add (Value)
    DemoTrace ("Add", Res)
    print ("trace end", Res)

if __name__ == '__main__':
    PyTraceInit ()
    DemoTr(8)


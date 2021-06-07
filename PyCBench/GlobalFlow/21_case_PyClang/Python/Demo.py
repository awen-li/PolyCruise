#!/usr/bin/python
import os
from PyDemo import *

if __name__ == '__main__':
    Cmd2 = os.getenv ("CTX")
    Ret = pyInCmp(Cmd2)
    print ("Cmd2 Ret = ", Ret)

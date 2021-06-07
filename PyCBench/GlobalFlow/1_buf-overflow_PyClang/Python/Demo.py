#!/usr/bin/python
import os
from PyDemo import *

if __name__ == '__main__':
    Oper = os.getenv ("BIN_OP")
    Oper = int (Oper)
    Ret = pyBinOp(Oper)
    print ("pyBinOp Ret = ", Ret)

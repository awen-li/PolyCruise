#!/usr/bin/python
import os
from PyDemo import *

if __name__ == '__main__':
    Var = os.getenv("CASE1")
    Var = int (Var)
    Ret = pyBinOp(Var)

	

#!/usr/bin/python
import os
from PyDemo import *

class Obj:
	def __init__(self, C):
		self.Value = C
		
def GetObj ():
	return Obj (5)

if __name__ == '__main__':
	O = GetObj ()
	Val = O.Value
	Ret = pyBinOp(Val)
	print ("pyBinOp Ret = ", Ret)
	

#!/usr/bin/python
import os
from PyDemo import *

class Child:
	def __init__(self, C):
		self.Value = C
		
	def Increase (self):
		return (self.Value + 1)
		
class Parent:
	def __init__(self, P):
		self.Child = Child (P)
		
	def Run (self):
		Val = self.Child.Increase ()
		Ret = pyBinOp(Val)
		print ("pyBinOp Ret = ", Ret)

if __name__ == '__main__':
	Oper = os.getenv ("BIN_OP")
	Oper = int (Oper)
	par = Parent (Oper)
	par.Run ()
    

#!/usr/bin/python

import time
from demo import add
import ast
import astunparse

code = '''
def Add(a, b):
    for i in range (5):
        c = AddbyC (a,b) * i
    result = c + 5
    print("Add result = %d" %result)
'''

def AddbyC (a, b):   
    return add (a,b)

def Add(a, b):
    for i in range (5):
        c = AddbyC (a,b) * i
    result = c + 5
    print("Add result = %d" %result)

if __name__ == '__main__':
    Add(1,2)
    print (ast.dump(ast.parse("Data = [F(i)  for i in List if i > 0]")))
    print(astunparse.unparse(ast.parse(code)))

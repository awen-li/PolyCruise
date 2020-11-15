#!/usr/bin/python

import time
from demo import add
import ast

def AddbyC (a, b):   
    return add (a,b)

def Add(a, b):
    vector = [AddbyC (a,b) * i for i in range (5)]
    c = 0
    for i in vector:
        c = c+i
    result = c/(a + b)
    print("Add result = %d" %result)
    return result

if __name__ == '__main__':
    Add (1,2)
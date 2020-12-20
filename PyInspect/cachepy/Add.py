import builtins
import time
from demo import add
import ast
def AddbyC(a, b):
    v1 = add(a, b)
    return v1
def Add(a, b):
    v2 = []
    v5 = 5
    v6 = range(v5)
    for (v3, v4) in builtins.enumerate(v6):
        i = v4
        v8 = AddbyC(a, b)
        v7 = (v8 * i)
        v2.append(v7)
    vector = v2
    v9 = 0
    c = v9
    for (v10, v11) in builtins.enumerate(vector):
        i = v11
        v12 = (c + i)
        c = v12
    v14 = (a + b)
    v13 = (c / v14)
    result = v13
    v16 = 'Add result = %d'
    v15 = (v16 % result)
    print(v15)
    return result
v18 = '__main__'
v17 = (__name__ == v18)
if v17:
    v19 = 1
    v20 = 2
    Add(v19, v20)

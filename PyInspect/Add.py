import builtins
import time
from demo import add
def Source():
    v1 = 3
    return v1
def AddbyC(a, b):
    v2 = add(a, b)
    return v2
def Add(a, b):
    v3 = []
    v6 = 5
    v7 = range(v6)
    for (v4, v5) in builtins.enumerate(v7):
        i = v5
        v9 = AddbyC(a, b)
        v8 = (v9 * i)
        v3.append(v8)
    vector = v3
    v10 = 0
    c = v10
    for (v11, v12) in builtins.enumerate(vector):
        i = v12
        v13 = (c + i)
        c = v13
    v15 = (a + b)
    v14 = (c / v15)
    result = v14
    v17 = 'Add result = %d'
    v16 = (v17 % result)
    print(v16)
    return result
def Entry():
    v18 = Source()
    S = v18
    v19 = 2
    Add(S, v19)
v21 = '__main__'
v20 = (__name__ == v21)
if v20:
    Entry()

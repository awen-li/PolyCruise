import numpy as np
import os

atype=[('a', int), ('b', int), ('c', int), ('d', int), ('e', int), ('f', int), ('g', int), ('h', int), ('i', int), ('j', int),
       ('k', int), ('l', int), ('m', int), ('n', int), ('o', int), ('p', int), ('q', int), ('r', int), ('s', int), ('t', int),
       ('u', int), ('v', int), ('w', int), ('x', int), ('y', int), ('z', int), ('1', int), ('2', int), ('3', int), ('4', int)]
shape30 = (2,2,2, 2,2,2, 2,2,2, 2,2,2, 2,2,2, 2,2,2, 2,2,2)

AList = []
for i in range (0, 20):
    print ("======================================")
    print ("[%d]" %i)
    print ("======================================")
    x = np.empty(shape30, dtype=atype)
    print (">>> x ->", np.shape(x), " - ", x.itemsize)
    AList.append (x)
    
    print (".")
    s = np.sort(x, order='a')
    print (">>> s ->", np.shape(s), " - ", s.itemsize)
    AList.append (s)




import numpy as np
import os

def GenArray ():
    shape30 = (2,2,2, 2,2,2, 2,2,2, 2,2,2, 2,2,2, 2,2,2, 2,2,2, 2,2,2, 2,2,2, 2,2,2)
    shape33 = (2,2,2, 2,2,2, 2,2,2, 2,2,2, 2,2,2, 2,2,2, 2,2,2, 2,2,2, 2,2,2, 2,2,2, 1,1,1, 1)
    x = np.ones(shape30, dtype=np.int16)
    print (np.shape(x), " - ", x.itemsize)

    np.save("array", x)
    y = np.load("array.npy")
    print (">>> y ->", np.shape(y), " - ", y.itemsize)

    z = x.reshape (shape33)
    print (">>> z ->", np.shape(z), " - ", z.itemsize)
    np.save("array33", z)

if not os.path.exists ("array33.npy"):
    GenArray ()

a = np.load("array33.npy")
print (">>> a ->", np.shape(a), " - ", a.itemsize)

sa = a [0:8]
print (">>> sa ->", np.shape(sa), " - ", sa.itemsize)

#################################################################
# results:
#
# *** stack smashing detected ***: <unknown> terminated
#
################################################################



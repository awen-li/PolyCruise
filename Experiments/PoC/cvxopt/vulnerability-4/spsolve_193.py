import math
from cvxopt import base, blas, lapack, cholmod, misc_solvers
from cvxopt.base import matrix, spmatrix
from PyDemo import Capsule_SetName, Capsule_New

A = spmatrix([10, 3, 5, -2, 5, 2], [0, 2, 1, 3, 2, 3], [0, 0, 1, 1, 2, 3])
X = spmatrix(1.0,range(4),range(4))

F = cholmod.symbolic(A)
cholmod.numeric(A, F)

# change the name, and change the control flow
Capsule_SetName (F, "CHOLMOD FACTOR YYYYYYYYYYYYY")
print (cholmod.spsolve(F, X))

# construct a fake CHOLMOD FACTOR
N = Capsule_New ("CHOLMOD FACTOR NNNNNNNNN")
print (N)
print (cholmod.spsolve(N, X))

import math
from cvxopt import base, blas, lapack, cholmod, misc_solvers
from cvxopt.base import matrix, spmatrix
from PyDemo import Capsule_SetName, Capsule_New

A = spmatrix([10, 3, 5, -2, 5, 2], [0, 2, 1, 3, 2, 3], [0, 0, 1, 1, 2, 3])
X = matrix(range(8), (4,2), 'd')

F = cholmod.symbolic(A)
cholmod.numeric(A, F)

# change the name, and change the control flow
Capsule_SetName (F, "CHOLMOD FACTOR YYYYYYYYYYYYY")
print (cholmod.solve(F, X))

# construct a fake CHOLMOD FACTOR
N = Capsule_New ("CHOLMOD FACTOR NNNNNNNNN")
print (cholmod.spsolve(N, X))

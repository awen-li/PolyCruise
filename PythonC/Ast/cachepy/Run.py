import builtins
import os
import sys
import getopt
import pandas as pd
from lib.NaiveBayes import NaiveBayes
from lib.CNNClf import CnnClf
import matplotlib.pyplot as plt

def main(argv):
    v1 = '1'
    Question = v1
    v3 = 'hq:'
    v5 = 'q='
    v4 = [v5]
    v6 = getopt.getopt(argv, v3, v4)
    v2 = v6
    v8 = 0
    v7 = v2[v8]
    opts = v7
    v10 = 1
    v9 = v2[v10]
    args = v9
    for (v11, v12) in builtins.enumerate(opts):
        v13 = v12
        v15 = 0
        v14 = v13[v15]
        opt = v14
        v17 = 1
        v16 = v13[v17]
        arg = v16
        v19 = '-h'
        v18 = (opt == v19)
        if v18:
            v20 = 'Run.py -q <question number>'
            print(v20)
            sys.exit()
        else:
            v23 = '-q'
            v24 = '--question'
            v22 = (v23, v24)
            v21 = (opt in v22)
            if v21:
                Question = arg
    v26 = os.path
    v27 = 'result'
    v28 = v26.exists(v27)
    v25 = (not v28)
    if v25:
        v29 = 'result'
        os.makedirs(v29)
    v31 = os.path
    v32 = 'data'
    v33 = v31.exists(v32)
    v30 = (not v33)
    if v30:
        v34 = 'data'
        os.makedirs(v34)
    v36 = '1'
    v35 = (Question == v36)
    if v35:
        v37 = 'stoplist.txt'
        v38 = NaiveBayes(v37)
        NB = v38
        v39 = 'traindata.txt'
        v40 = 'trainlabels.txt'
        v41 = 'testdata.txt'
        v42 = 'testlabels.txt'
        NB.Test(v39, v40, v41, v42)
    else:
        v43 = CnnClf()
        Cnn = v43
        Cnn.TrainCnn()
v45 = '__main__'
v44 = (__name__ == v45)
if v44:
    v47 = sys.argv
    v48 = 1
    v46 = v47[v48:]
    main(v46)

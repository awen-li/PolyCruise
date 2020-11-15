#!/usr/bin/python

import os
import sys, getopt
import pandas as pd
from lib.NaiveBayes import NaiveBayes
from lib.CNNClf import CnnClf
import matplotlib.pyplot as plt


def main(argv):
    Question = "1"
    
    opts, args = getopt.getopt(argv,"hq:",["q="])

    for opt, arg in opts:
        if opt == '-h':
            print ("Run.py -q <question number>")
            sys.exit()
        elif opt in ("-q", "--question"):
            Question = arg;

    if (not os.path.exists("result")):
        os.makedirs("result")

    if (not os.path.exists("data")):
        os.makedirs("data")


    if (Question == "1"):
        NB = NaiveBayes ("stoplist.txt")
        NB.Test ("traindata.txt", "trainlabels.txt", "testdata.txt", "testlabels.txt")
    else:
        Cnn = CnnClf ()
        Cnn.TrainCnn ()
   

if __name__ == "__main__":
   main(sys.argv[1:])

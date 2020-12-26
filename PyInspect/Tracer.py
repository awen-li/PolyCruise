#!/usr/bin/python

import os
import sys, getopt
import marshal
from Inspector import Inspector
from ModRewriter import PyRecompile
from os.path import join, abspath, splitext, realpath
from Add import Add

def main(argv):
    Action = ""
    File = ""
    
    try:
        opts, args = getopt.getopt(argv,"ha:f:",["a="])
    except getopt.GetoptError:
        print ("Tracer.py -a <action>")
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print ("Tracer.py -a <action>")
            sys.exit()
        elif opt in ("-a", "--action"):
            Action = arg;
        elif opt in ("-f", "--filename"):
            File = arg;

    if File == "":
        return
  
    if Action == "recomp":
        PyRecompile (File)
    elif Action == "trace":
        PyFile = abspath(File)
        with open(PyFile, 'rb') as fp:
            Code = fp.read()
            #print (Code)
        
        Globs = {
            '__file__': PyFile,
            '__name__': '__main__',
            '__package__': None,
            '__cached__': None,
        }

        #print (Globs)
        with Inspector ("pyList"):
            #exec (Code, Globs)
            Add (1,2)
        print('tracing finished...')

if __name__ == "__main__":
   main(sys.argv[1:])
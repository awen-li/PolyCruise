#!/usr/bin/python

import os
import sys, getopt
from lib.xmlGen import xmlGen
from lib.xmlExtr import xmlExtr

def main(argv):
    Dir = ""

    os.system ("cp tool/bin/* /usr/bin/")
    os.system ("cp tool/lib/* /usr/lib/")
    
    try:
        opts, args = getopt.getopt(argv,"d:",["d="])
    except getopt.GetoptError:
        print ("srcXml.py -d <dirname>")
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-d':
            Dir = arg;
    
    if Dir == "":
        print ("srcXml.py -d <dirname>")
        return
  
    XG = xmlGen (Dir)
    XG.Gen ()

    XE = xmlExtr ()
    XE.Extr ()

if __name__ == "__main__":
   main(sys.argv[1:])
#!/usr/bin/python

import os
import sys, getopt
import marshal
from .Inspector import Inspector
from .ModRewriter import PyRecompile
from os.path import join, abspath, splitext, realpath

ROOTDIR = "Temp"
PyList  = ROOTDIR + "/pyList"

def __MakeDir (Dir):
    if os.path.exists (Dir):
        return
    os.mkdir (Dir)

def __Copy (Dir):
    RootDir = ROOTDIR
    __MakeDir (RootDir)
    CpCmd = "cp -rf " + Dir + " " + RootDir + "/" + Dir
    os.system (CpCmd)

def PyTranslateFile (PyFile):
    __Copy (PyFile)
    PyRecompile (PyFile, ROOTDIR)
    with open(PyList, "w") as File:
        _, Name = os.path.split(PyFile)
        File.write(Name + "\n")

def PyTranslate (PyDir):
    __Copy (PyDir)

    PyLists = []
    PyDirs = os.walk(PyDir) 
    for Path, Dirs, Pys in PyDirs:
        for py in Pys:
            _, Ext = os.path.splitext(py)
            if Ext != ".py":
                continue
            
            PyFile = os.path.join(Path, py)
            PyRecompile (PyFile, ROOTDIR)
            
            _, Name = os.path.split(py)
            PyLists.append (Name)
    
    with open(PyList, "w") as File:
        for Py in PyLists:
            File.write(Py + "\n")
    
    return


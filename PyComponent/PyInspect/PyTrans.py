#!/usr/bin/python

import os
import sys, getopt
import marshal
from ast import parse
from .Inspector import Inspector
from .ModRewriter import PyRecompile, RelatPath
from .AstVisit import ASTWalk
from os.path import join, abspath, splitext, realpath
from xml.dom.minidom import Document


ROOTDIR = "Temp"

def __MakeDir (Dir):
    if os.path.exists (Dir):
        return
    #os.mkdir (Dir)
    Cmd = "mkdir -p " + Dir
    print (Cmd)
    os.system (Cmd)

def __Copy (Dir):
    Target = ROOTDIR + "/"   
    __MakeDir (Target)
    CpCmd = "cp -rf " + Dir + " " + Target
    os.system (CpCmd)

def PyTranslateFile (PyFile):
    __Copy (PyFile)
    PyRecompile (PyFile, ".", ROOTDIR)
    with open(PyList, "w") as File:
        _, Name = os.path.split(PyFile)
        File.write(Name + "\n")

def IsInExpList (py, PyFile, ExpList):
    if ExpList == None:
        return False
    if py in ExpList:
        return True
    for exp in ExpList:
        Hd = exp[0:2]
        if Hd != "-D":
            continue
        if PyFile.find (exp[2:]) != -1:
            return True
    return False
        
def PyTranslate (PyDir, ExpList=None):
    __Copy (PyDir)

    PyLists = []
    PyDirs = os.walk(PyDir) 
    for Path, Dirs, Pys in PyDirs:
        for py in Pys:
            _, Ext = os.path.splitext(py)
            if Ext != ".py":
                continue

            PyFile = os.path.join(Path, py)
            if IsInExpList (py, PyFile, ExpList) == True:
                continue    
            
            PyRecompile (PyFile, PyDir, ROOTDIR)

            RelativePath = RelatPath (PyFile, PyDir)
            PyLists.append (RelativePath)

    PYLIST  = ROOTDIR + "/" + PyDir + "/pyList"
    with open(PYLIST, "w") as File:
        for Py in PyLists:
            File.write(Py + "\n")
    return

def _AddChildNode (Doc, Parent, Child, Value=None):
    CNode = Doc.createElement(Child)
    Parent.appendChild(CNode)
    if Value != None:
        Val = Doc.createTextNode(Value)
        CNode.appendChild(Val)
    return CNode
    

def PyGenSource (PyDir):
    doc = Document()  
    Crit = _AddChildNode (doc, doc, "criterions")

    PyLists = []
    PyDirs = os.walk(PyDir) 
    for Path, Dirs, Pys in PyDirs:
        for py in Pys:
            _, Ext = os.path.splitext(py)
            if Ext != ".py":
                continue
            
            if py[0:5] != "test_":
                continue
            
            PyFile = os.path.join(Path, py)
            print (PyFile)
            
            with open(PyFile) as PyF:
                Ast = parse(PyF.read(), PyFile, 'exec')
                Visitor= ASTWalk()
                Visitor.visit(Ast)

                FuncDef = Visitor.FuncDef
                for FuncName, Tag in FuncDef.items ():
                    Src = _AddChildNode (doc, Crit, "criterion")
                    _AddChildNode (doc, Src, "function", FuncName)
                    _AddChildNode (doc, Src, "return", "False")
                    _AddChildNode (doc, Src, "local", "11111111")
    
    f = open("gen_criterion.xml", "w")
    f.write(doc.toprettyxml(indent="  "))
    f.close()
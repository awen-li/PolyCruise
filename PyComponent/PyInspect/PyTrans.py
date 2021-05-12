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
import pickle

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
    

def PyGenSource (PyDir, ExpList=None, Names=None):
    doc = Document()  
    Crit = _AddChildNode (doc, doc, "criterions")

    SrcApiList = {}
    FuncDefList = {}
    
    PyDirs = os.walk(PyDir) 
    for Path, Dirs, Pys in PyDirs:
        for py in Pys:
            _, Ext = os.path.splitext(py)
            if Ext != ".py":
                continue
         
            PyFile = os.path.join(Path, py)
            if IsInExpList (py, PyFile, ExpList) == True:
                continue  

            Prefix = py[0:5]
            with open(PyFile) as PyF:
                Ast = parse(PyF.read(), PyFile, 'exec')
                Visitor= ASTWalk(PyDir, Names)
                Visitor.visit(Ast)
                # source api retrieve
                if Prefix == "test_":
                    SrcApi = Visitor.SrcApiDef
                    for FuncName, Tag in SrcApi.items ():
                        SrcApiList[FuncName] = Tag
                # function definition retrieve
                FuncDef = Visitor.FuncDef
                for FuncName, FDef in FuncDef.items ():
                    FDef.Id = len (FuncDefList)+2
                    FuncDefList[FuncName] = FDef

    for FuncName, Tag in SrcApiList.items ():
        Src = _AddChildNode (doc, Crit, "criterion")
        _AddChildNode (doc, Src, "function", FuncName)
        _AddChildNode (doc, Src, "return", "False")
        _AddChildNode (doc, Src, "local", "11111111")
    
    f = open(PyDir+"_gen_criterion.xml", "w")
    f.write(doc.toprettyxml(indent="  "))
    f.close()
    print ("SrcApiList size = ", len (SrcApiList))

    # write function def
    FDefPkl = "function_def.pkl"
    with open(FDefPkl, 'wb') as Pkl:
        pickle.dump(FuncDefList, Pkl)
        print ("FuncDefList size = ", len (FuncDefList))
    FDefPklTxt = "function_def.pkl.txt"
    with open(FDefPklTxt, 'w') as PklTxt:
        for Name, Def in FuncDefList.items ():
            PklTxt.write(Name + "\n")

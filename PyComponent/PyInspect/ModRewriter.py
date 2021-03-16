#!/usr/bin/python

import os
import ast
from ast import parse
from os.path import abspath, sep, join
import astunparse
import pickle
from .AstRewriter import ASTVisitor

def RelatPath (File, PrjName):
    Index = File.find (PrjName) + len (PrjName) + 1
    Relat = File [Index:]
    return Relat
    
class NewASTInfo(object):
    def __init__(self, filename, lineno2stmt, newlineno2oldlineno):
        self.filename = filename
        self.lineno2stmt = lineno2stmt
        self.newlineno2oldlineno = newlineno2oldlineno
        if filename.find ("runtests") != -1:
            self.dump(filename+"-debug.txt")

    def dump (self, Name):
        with open(Name, 'w') as File:
            for line, stmt in self.lineno2stmt.items ():
                File.write("line-" + str(line) + ", stmt-" + ast.dump (stmt) + "\n")

class PyRecompile (object):
    def __init__(self, PyFile, PrjDir, OutDir="."):
        self.RecompilePyFile (PyFile, PrjDir, OutDir)

    def RecompilePyFile(self, SrcFile, PrjName, OutDir='.'):        
        print ("Recompile python source: ", SrcFile)
        with open(SrcFile) as Pyfile:
            OrgAst = parse(Pyfile.read(), SrcFile, 'exec')
        Visitor= ASTVisitor(nestedexpand=True)
        NewAst = Visitor.visit(OrgAst)
        NewAstInfo = NewASTInfo(SrcFile, Visitor.lineno2stmt, Visitor.newlineno2oldlineno)
        #print (ast.dump (OrgAst))
        #print (ast.dump (NewAst))

        DestFile = OutDir + "/" + SrcFile
        print ("\t => Generate new python source: ", DestFile)
        with open(DestFile, 'w') as NewPyfile:
            #print (NewAst.__class__.__name__)
            source = astunparse.unparse(NewAst)  
            if source.startswith('\n'):
                source = source[1:]
            while (True):
                BlankLine = source.find ("\n\n")
                if BlankLine != -1:
                    NewPyfile.write(source[0:BlankLine+1])
                    source = source [BlankLine+2:]
                else:
                    NewPyfile.write(source)
                    break

        # write the pickle files
        PkFile = RelatPath (SrcFile, PrjName)
        EncodePkFile = str(PkFile).replace(sep, '#')
        PklPath = OutDir + "/" + PrjName + '/cachepkl'
        if not os.path.exists(PklPath):
            os.mkdir(PklPath)
        PklName = os.path.join(PklPath, EncodePkFile + '.pkl')
        with open(PklName, 'wb') as Pkl:
            pickle.dump(NewAstInfo, Pkl)
        astunparse.dump(NewAst)
        
        return NewAst
    


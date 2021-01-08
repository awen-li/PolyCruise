#!/usr/bin/python

import os
import ast
from ast import parse
from os.path import abspath, sep, join
import astunparse
import pickle
from .AstRewriter import ASTVisitor


def encode_filename(filename):
    return str(filename).replace(sep, '#')
    
class NewASTInfo(object):
    def __init__(self, filename, lineno2stmt, newlineno2oldlineno):
        self.filename = filename
        self.lineno2stmt = lineno2stmt
        self.newlineno2oldlineno = newlineno2oldlineno

class PyRecompile (object):
    def __init__(self, PyFile, OutDir="."):
        self.RecompilePyFile (PyFile, OutDir)

    def RecompilePyFile(self, filename, tmpdir='.'):
        print ("Recompile python source: ", filename)
        with open(filename) as pyfile:
            ori_ast = parse(pyfile.read(), filename, 'exec')
        visitor = ASTVisitor(nestedexpand=True)
        new_ast = visitor.visit(ori_ast)
        newast_info = NewASTInfo(filename, visitor.lineno2stmt, visitor.newlineno2oldlineno)
        #print (ast.dump (ori_ast))
        #print (ast.dump (new_ast))

        newsource_filename = tmpdir + "/" + filename
        print ("\t => Generate new python source: ", newsource_filename)
        with open(newsource_filename, 'w') as sourcefile:
            #print (new_ast.__class__.__name__)
            source = astunparse.unparse(new_ast)  
            if source.startswith('\n'):
                source = source[1:]
            while (True):
                BlankLine = source.find ("\n\n")
                if BlankLine != -1:
                    sourcefile.write(source[0:BlankLine+1])
                    source = source [BlankLine+2:]
                else:
                    sourcefile.write(source)
                    break

        # write the pickle files
        Path, Name = os.path.split(filename)
        cachepklpath = os.path.join(tmpdir, 'cachepkl')
        if not os.path.exists(cachepklpath):
            os.mkdir(cachepklpath)
        pkl_filename = os.path.join(cachepklpath, Name+'.pkl')
        with open(pkl_filename, 'wb') as pklfile:
            pickle.dump(newast_info, pklfile)
        astunparse.dump(new_ast)
        return new_ast
    

#recompile_pyfile ("Add.py")

#!/usr/bin/python

import os
import ast
from ast import parse
from os.path import abspath, sep, join
import astunparse
from AstRewriter import ASTVisitor
import pickle

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
        with open(filename) as pyfile:
            ori_ast = parse(pyfile.read(), filename, 'exec')
        visitor = ASTVisitor(nestedexpand=True)
        new_ast = visitor.visit(ori_ast)
        newast_info = NewASTInfo(filename, visitor.lineno2stmt, visitor.newlineno2oldlineno)

        # only for debug the astrewriter module
        cachepydir = os.path.join(tmpdir, 'cachepy')
        if not os.path.exists(cachepydir):
            os.makedirs(cachepydir)
        newsource_filename = os.path.join(cachepydir, encode_filename(filename))
        #print (ast.dump (ori_ast))
        #print (ast.dump (new_ast))

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
        cachepklpath = os.path.join(tmpdir, 'cachepkl')
        if not os.path.exists(cachepklpath):
            os.mkdir(cachepklpath)
        pkl_filename = os.path.join(cachepklpath, encode_filename(filename)+'.pkl')
        with open(pkl_filename, 'wb') as pklfile:
            # print '{filename}\n ' \
            #       '[lineno2stmt]:\n' \
            #       '{ls}'.format(filename=newast_info.filename,
            #                     ls='\n'.join('{k}:{v}'.format(k=k,
            #                                                   v=dump(v))
            #                     for k, v in newast_info.lineno2stmt.iteritems()))
            pickle.dump(newast_info, pklfile)
        astunparse.dump(new_ast)
        return new_ast
    

#recompile_pyfile ("Add.py")

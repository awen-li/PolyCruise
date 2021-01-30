#!/usr/bin/python

import os
import sys, getopt
import argparse
import xml.dom.minidom
from xml.dom.minidom import parse
from PyInspect import Inspector 
from PyInspect import PyTranslate, PyTranslateFile
from PyInspect import Criterion


class xmlParse ():
    def __init__(self, CriteCfg="criterion.xml"):
        self.CriteCfg    = CriteCfg
        self.Directory   = ""
        self.EntryScript = ""
        self.Critn = Criterion ()
        self.Parse (CriteCfg)

    def InitCrite (self, Func, Paras):
        print ("Init critesion: %s:%s" %(Func, Paras))
        TaintBits = []
        if Paras != "NULL":
            List = Paras.split()
            for Val in List:
                TaintBits.append (int (Val))
        self.Critn.Insert (Func, TaintBits)

    def ParseCrite (self, Crite):
        Func  = Crite.getElementsByTagName('function')[0].childNodes[0].data
        Paras = Crite.getElementsByTagName("parameters")[0].childNodes[0].data
        return Func, Paras

    def Parse (self, xmlFile):
        DOMTree  = xml.dom.minidom.parse(xmlFile)
        CriteDoc = DOMTree.documentElement

        self.Directory   = CriteDoc.getAttribute("code_directory")
        self.EntryScript = CriteDoc.getAttribute("entry_script")
        print ("[xml]", self.Directory, ":", self.EntryScript)
         
        # iterate all criterions
        AllCrites = CriteDoc.getElementsByTagName("criterion")
        for Crite in AllCrites:
            Func, Paras = self.ParseCrite (Crite)
            self.InitCrite (Func, Paras)

def ParseExpList (ExpFile):
    ExpList = []
    with open(ExpFile, 'r', encoding='latin1') as exfile:
        for line in exfile:
            ExpList = ExpList + list (line.split ())
    return ExpList

def InitArgument (parser):
    parser.add_argument('--version', action='version', version='trace 2.0')
    
    grp = parser.add_argument_group('Main options', 'One of these (or --report) must be given')
    grp.add_argument('-d', '--directory',
                     help='process all files in the directory')
    grp.add_argument('-c', '--compile', action='store_true',
                     help='recompile the python source ')
    grp.add_argument('-t', '--trace', action='store_true',
                     help='trace the program during runtime ')
    grp.add_argument('-C', '--criterion',
                     help='the xml file for defining criterion ')
    grp.add_argument('-E', '--exceptfile',
                     help='the configure file for elimiate unnecesssay py files')
                     
    parser.add_argument('filename', nargs='?', help='file to run as main program')
    parser.add_argument('arguments', nargs=argparse.REMAINDER, help='arguments to the program')
    
    
def RunCtx(Cmd, globals=None, locals=None):
    if globals == None: 
        globals = {}
    if locals == None: 
        locals = {}
    exec(Cmd, globals, locals)
    

def Recompile (Dir, ExpList=None):
    if os.path.isdir(Dir):
        PyTranslate (Dir, ExpList)
    elif os.path.isfile(Dir):
        PyTranslateFile (Dir)
    else:
        print ("unknow file....")
    

def DynTrace (EntryScript, CriteCfg):
    XP = xmlParse (CriteCfg)
    
    try:
        with open(EntryScript) as fp:
            code = compile(fp.read(), EntryScript, 'exec')
        # try to emulate __main__ namespace as much as possible
        globs = {
            '__file__': EntryScript,
            '__name__': '__main__',
            '__package__': None,
            '__cached__': None,
        }
        
        with Inspector ("pyList", XP.Critn):
            RunCtx(code, globs, globs)
        
    except OSError as err:
        sys.exit("Cannot run file %r because: %s" % (sys.argv[0], err))
    except SystemExit:
        sys.exit("except SystemExit")

def main():
    parser = argparse.ArgumentParser()
    InitArgument (parser)

    opts = parser.parse_args()  
    if opts.compile == True:
        ExpList=["setup.py", "__init__.py", "build.py"]
        if opts.exceptfile != None:
            ExpList = ParseExpList (opts.exceptfile)

        if opts.directory != None:
            Recompile (opts.directory, ExpList)
        else:
            if opts.filename is None:
                parser.error('filename is missing: required with the main options')
            Recompile (opts.filename)
    elif opts.trace == True:
        sys.argv = [opts.filename, *opts.arguments]
        sys.path[0] = os.path.dirname(opts.filename)

        if opts.filename is None:
            parser.error('filename is missing: required with the main options')
        if opts.criterion is None:
            parser.error('critefion xml is missing: required with the main options')
        
        DynTrace (opts.filename, opts.criterion)
    else:
        print ("do nothing?") 

    print ("Run successful.....")

if __name__ == "__main__":
   main()

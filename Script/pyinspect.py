#!/usr/bin/python

import os
import sys, getopt
import argparse
import xml.dom.minidom
from xml.dom.minidom import parse
from PyInspect import Inspector 
from PyInspect import PyTranslate, PyTranslateFile, PyGenSource
from PyInspect import Criterion

PY_MAPING = "Pymap.ini"

class PyMaping ():  
    def __init__(self, IniSet="*.ini", SrcSet="pyList"):
            IniFiles = self.LoadFile(IniSet)
            SrcFiles = self.LoadFile(SrcSet)
            PyMap = self.Maping (IniFiles, SrcFiles)
            self.Dump (PyMap)

    def LoadFile (self, Name):
        Content = []
        with open(Name, 'r', encoding='latin1') as File:
            for line in File:
                line = line.strip('\n')
                Content.append (line)
        return Content

    def Dump (self, PyMap):
        with open(PY_MAPING, "w") as File:
            for ini, src in PyMap.items():
                File.write(ini + " " + src + "\n")

    def MaxPrefix(self, StrList):
        Max = ""    
        for S in zip(*StrList):        
            if len(set(S))==1:            
                Max +=S[0]        
            else:            
                break    
        return Max

    def IsValidPy (self, Py):
        with open(Py, 'r', encoding='latin1') as File:
            Line = 0
            for line in File:
                if line.find ("__bootstrap__") != -1:
                    return False
                Line += 1
                if Line >= 2:
                    break;
        return True

    def Maping (self, IniFiles, SrcFiles):
        PyMap = {}
        for ini in IniFiles:
            if not self.IsValidPy (ini):
                continue
            IniF = ini[::-1]
            Max  = ""
            MaxSrc = ""
            for src in SrcFiles:
                srcF = src[::-1]
                StrList = [IniF, srcF]
                Prefix = self.MaxPrefix (StrList)
                if len (Prefix) > len (Max):
                    Max = Prefix
                    MaxSrc = src
            DirList = MaxSrc.split("/")
            if len (DirList) >= 2 and ini.find (DirList[-2]) != -1:
                print (ini, " -> ", MaxSrc)
                PyMap[ini] = MaxSrc
        return PyMap

class xmlParse ():
    def __init__(self, CriteCfg="criterion.xml"):
        self.CriteCfg    = CriteCfg
        self.Directory   = ""
        self.EntryScript = ""
        self.Critn = Criterion ()
        self.Parse (CriteCfg)

    def InitCrite (self, Func, Return, Local):
        #print ("Init critesion: %s:%s:%s" %(Func, Return, Local))
        self.Critn.Insert (Func, Return, Local)

    def ParseCrite (self, Crite):
        Func   = Crite.getElementsByTagName('function')[0].childNodes[0].data
        Return = Crite.getElementsByTagName("return")[0].childNodes[0].data
        Local  = Crite.getElementsByTagName("local")[0].childNodes[0].data
        return Func, Return, Local

    def Parse (self, xmlFile):
        DOMTree  = xml.dom.minidom.parse(xmlFile)
        CriteDoc = DOMTree.documentElement

        self.Directory   = CriteDoc.getAttribute("code_directory")
        self.EntryScript = CriteDoc.getAttribute("entry_script")
        print ("[xml]", self.Directory, ":", self.EntryScript)
         
        # iterate all criterions
        AllCrites = CriteDoc.getElementsByTagName("criterion")
        for Crite in AllCrites:
            Func, Return, Local = self.ParseCrite (Crite)
            self.InitCrite (Func, Return, Local)

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
    grp.add_argument('-M', '--maping',
                     help='maping the installed source py files')
    grp.add_argument('-g', '--gen_source',
                     help='generate possible sources')
                     
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
        
        with Inspector ("pyList", XP.Critn, PY_MAPING):
            RunCtx(code, globs, globs)
        
    except OSError as err:
        sys.exit("Cannot run file %r because: %s" % (sys.argv[0], err))
    except SystemExit:
        sys.exit("except SystemExit")

def main():
    parser = argparse.ArgumentParser()
    InitArgument (parser)

    opts = parser.parse_args()
    if opts.gen_source != None:
        print ("Start generate possible sources")
        ExpList = None
        if opts.exceptfile != None:
            ExpList = ParseExpList (opts.exceptfile)
        PyGenSource (opts.gen_source, ExpList)
    elif opts.maping != None:
        if opts.filename is None:
            parser.error('filename is missing: required with the main options')
        PyMaping (opts.maping, opts.filename)
    elif opts.compile == True:
        ExpList=None
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

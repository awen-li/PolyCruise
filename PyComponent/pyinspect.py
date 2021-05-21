#!/usr/bin/python

import os
import sys, getopt
import argparse
import time
import xml.dom.minidom
from xml.dom.minidom import parse
from PyInspect import Inspector
from PyInspect import Trace
from PyInspect import PyTranslate, PyTranslateFile, PyGenSource
from PyInspect import Criterion

PY_MAPING = "Pymap.ini"

 
InitTicks = time.time()

def TIME_COST (Name):
    print ("@@@@ ", Name, " time cost: ", str (time.time() - InitTicks))

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
            if len (DirList) >= 2 and ini.find (DirList[-2]) != -1 and ini.find (DirList[-1]) != -1:
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
        #print ("[xml]", self.Directory, ":", self.EntryScript)
         
        # iterate all criterions
        AllCrites = CriteDoc.getElementsByTagName("criterion")
        for Crite in AllCrites:
            Func, Return, Local = self.ParseCrite (Crite)
            self.InitCrite (Func, Return, Local)

def ParseText (TxtFile):
    Content = []
    with open(TxtFile, 'r', encoding='latin1') as txfile:
        for line in txfile:
            Content = Content + list (line.split ())
    return Content


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
    grp.add_argument('-n', '--name_modules',
                     help='names of modules')
    grp.add_argument('-p', '--print_trace', action='store_true',
                     help='print the traces')
    grp.add_argument('-i', '--include_path',
                     help='the installation path of the target')
                     
    parser.add_argument('filename', nargs='?', help='file to run as main program')
    parser.add_argument('arguments', nargs=argparse.REMAINDER, help='arguments to the program')
    
    
def RunCtx(Cmd, globals=None, locals=None):
    if globals == None: 
        globals = {}
    if locals == None: 
        locals = {}
    exec(Cmd, globals, locals)
    #TIME_COST ("RunCtx")
    

def Recompile (Dir, ExpList=None):
    if os.path.isdir(Dir):
        PyTranslate (Dir, ExpList)
    elif os.path.isfile(Dir):
        PyTranslateFile (Dir)
    else:
        print ("unknow file....")
    

def DynTrace (EntryScript, CriteCfg):
    XP = xmlParse (CriteCfg)
    #TIME_COST ("Inspect-xmlParse")
    
    try:
        with open(EntryScript) as fp:
            code = compile(fp.read(), EntryScript, 'exec')

        #TIME_COST ("Inspect-Compile")
        # try to emulate __main__ namespace as much as possible
        globs = {
            '__file__': EntryScript,
            '__name__': '__main__',
            '__package__': None,
            '__cached__': None,
        }
        
        with Inspector ("pyList", XP.Critn, PY_MAPING):
            #TIME_COST ("Inspect-Init")
            RunCtx(code, globs, globs)
        
    except OSError as err:
        sys.exit("Cannot run file %r because: %s" % (sys.argv[0], err))
    except SystemExit:
        sys.exit("except SystemExit")

def PrintTrace (EntryScript, Module):
    try:
        with open(EntryScript) as fp:
            code = compile(fp.read(), EntryScript, 'exec')

        #TIME_COST ("Inspect-Compile")
        # try to emulate __main__ namespace as much as possible
        globs = {
            '__file__': EntryScript,
            '__name__': '__main__',
            '__package__': None,
            '__cached__': None,
        }
        
        with Trace (Module):
            exec(code, globs, globs)
        
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
        Names = None
        if opts.exceptfile != None:
            ExpList = ParseText (opts.exceptfile)
        if opts.name_modules != None:
            Names = ParseText (opts.name_modules)
        PyGenSource (opts.gen_source, ExpList, Names)
    elif opts.maping != None:
        if opts.filename is None:
            parser.error('filename is missing: required with the main options')
        PyMaping (opts.maping, opts.filename)
    elif opts.compile == True:
        ExpList=None
        if opts.exceptfile != None:
            ExpList = ParseText (opts.exceptfile)

        if opts.directory != None:
            Recompile (opts.directory, ExpList)
        else:
            if opts.filename is None:
                parser.error('filename is missing: required with the main options')
            Recompile (opts.filename)
    elif opts.trace == True:
        sys.argv = [opts.filename, *opts.arguments]
        if opts.include_path is None:
            default_path = os.path.abspath(r".")
            sys.path.insert (0, default_path)
            print ("@@@ add default sys path:", default_path)     
        else:
            sys.path.insert (0, opts.include_path)
            print ("@@@ add sys path:", opts.include_path)

        if opts.filename is None:
            parser.error('filename is missing: required with the main options')
        if opts.criterion is None:
            parser.error('critefion xml is missing: required with the main options')
        
        DynTrace (opts.filename, opts.criterion)
    elif opts.print_trace == True:
        sys.argv = [opts.filename, *opts.arguments]

        if opts.include_path is None:
            default_path = os.path.abspath(r".")
            sys.path.insert (0, default_path)
            print ("@@@ add default sys path:", default_path)     
        else:
            sys.path.insert (0, opts.include_path)
            print ("@@@ add sys path:", opts.include_path)

        if opts.filename is None:
            parser.error('filename is missing: required with the main options')
        if opts.name_modules is None:
            parser.error('needs the name of the module')
        PrintTrace (opts.filename, opts.name_modules)
    else:
        print ("do nothing?") 

    print ("Run successful.....")

if __name__ == "__main__":
   main()

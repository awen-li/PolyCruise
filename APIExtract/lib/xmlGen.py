#!/usr/bin/python

import sys
import os
import shutil


class xmlGen ():
    def __init__(self, Dir, Output="xmls/"):
        self.Dir    = Dir
        self.Output = Output
        if os.path.exists (Output):
            shutil.rmtree (Output)
        os.makedirs(Output)

    def Gen (self):
        SrcDirs = os.walk(self.Dir) 
        for Path, Dirs, Srcs in SrcDirs:
            for src in Srcs:
                if os.path.splitext(src)[-1] != ".c":
                    continue
                
                SrcFile = os.path.join(Path, src)
                xmlFile = self.Output + src + ".xml"
                print (SrcFile, " ------> ", xmlFile)
                Command = "srcml " + SrcFile + " > " + xmlFile
                os.system (Command)
  

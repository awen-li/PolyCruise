#!/usr/bin/python

import sys
import inspect
import threading

from Analyzer import Analyzer

class Inspector:
    def __init__(self, RecordFile):
        print ("----> __init__................")
        self.Analyzer = Analyzer (RecordFile)

    def __enter__(self):
        print ("----> __enter__................")
        threading.settrace(self.Tracing)
        sys.settrace(self.Tracing)    
        return self

    def __exit__(self, *_):
        print ("----> __exit__................")
        sys.settrace(None)
        threading.settrace(None)

    def Tracing(self, Frame, Event, Arg):        
        Code = Frame.f_code
        Module = inspect.getmodule(Code)
        ModuleName = ""
        ModulePath = ""

        if Module:
            ModulePath = Module.__file__
            ModuleName = Module.__name__
        else:
            return

        if ModuleName == "Inspector":
            return
        
        LineNo = Frame.f_lineno
        print(LineNo, Event, Code.co_name, end=" => ")
        Def, Use = self.Analyzer.HandleEvent (ModulePath, Frame, Event, LineNo)
        print ("---> Def: ", Def, " Use: ", Use)

        return self.Tracing



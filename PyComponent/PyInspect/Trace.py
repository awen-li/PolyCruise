#!/usr/bin/python

import os
import sys
import inspect

try:
    import threading
except ImportError:
    _settrace = sys.settrace

    def _unsettrace():
        sys.settrace(None)
else:
    def _settrace(func):
        threading.settrace(func)
        sys.settrace(func)

    def _unsettrace():
        sys.settrace(None)
        threading.settrace(None)

class Trace:
    def __init__(self, Target):
    	self.Target = Target

    def __enter__(self):
        print ("----> Trace:__enter__................")
        _settrace(self.Tracing)
        return self

    def __exit__(self, *_):
        print ("----> Trace:__exit__................")
        _unsettrace()

    def Tracing(self, Frame, Event, Arg):    
        Code = Frame.f_code
        File = Code.co_filename

        if File.find (self.Target) == -1:
            return self.Tracing
            
        print (Code.co_filename, ":", Frame.f_lineno, ":", Event, " -> ", Code.co_name)
        return self.Tracing



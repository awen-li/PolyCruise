#!/usr/bin/python

import sys
import inspect
import threading
from PyTrace import *
from Criterion import Criterion
from Analyzer import Analyzer

EVENT_DF     = 0
EVENT_FENTRY = 1
EVENT_NR     = 2
EVENT_BR     = 3
EVENT_RET    = 4
EVENT_CALL   = 5
EVENT_THRC   = 6
EVENT_GEP    = 7
EVENT_STORE  = 8

class Context:
    def __init__(self, CurFunc, TaintInPara):
        self.Func = CurFunc
        self.TaintLexical = {}
        self.TaintInpara = TaintInPara
        self.TaintOutpara = []

    def InsertLexicon (self, Lexicon):
        self.TaintLexical [Lexicon] = True
        print ("self.TaintLexical = ", self.TaintLexical)

    def IsTaint (self, Lexicon):
        Flag = self.TaintLexical.get (Lexicon)
        if Flag == None:
            return False
        else:
            return True

class Inspector:
    def __init__(self, RecordFile):
        print ("----> __init__................")
        self.Analyzer = Analyzer (RecordFile)
        self.Crtn  = Criterion ()
        self.CtxStack = []
        self.Buildins = ["abs", "delattr", "hash", "memoryview", "set", "all", "dict", "help",
                         "min", "setattr", "any", "dir", "hex", "next", "slice", "ascii", "divmod",
                         "id", "object", "sorted", "bin", "enumerate", "input", "oct", "staticmethod", 
                         "bool", "eval", "int", "open", "str", "breakpoint", "exec", "isinstance", 
                         "ord", "sum", "bytearray", "filter", "issubclass", "pow", "super", "bytes", 
                         "float", "iter", "print", "tuple", "callable", "format", "len", "property", 
                         "type", "chr", "frozenset", "list", "range", "vars", "classmethod", "getattr", 
                         "locals", "repr", "zip", "compile", "globals", "map", "reversed", "complex", 
                         "hasattr", "max", "round"]

        # init main ctx
        self.CallCtx = None
        self.CurCtx  = Context ("main", [])
        self.CtxStack.append (self.CurCtx)
        print ("-----------> Push Context: ", self.CurCtx.Func)

    def __enter__(self):
        print ("----> __enter__................")
        PyTraceInit ()
        threading.settrace(self.Tracing)
        sys.settrace(self.Tracing)    
        return self

    def __exit__(self, *_):
        print ("----> __exit__................")
        sys.settrace(None)
        threading.settrace(None)
        PyTraceExit ()

    def GetTaintedParas (self, LiveObj):
        Uses = LiveObj.Uses
        TaintSet = []
        for Index in range (len(Uses)):
            use = Uses[Index]
            if self.CurCtx.IsTaint (use):
                TaintSet.append (Index)
        return TaintSet

    def SetCallCtx (self, Func, LiveObj):
        if self.Crtn.IsCriterion (LiveObj.Callee):
            self.CurCtx.InsertLexicon (LiveObj.Def)
            print ("****************<> Add source: ", LiveObj.Def, " = ", LiveObj.Callee)
        print (self.CurCtx.TaintLexical)        
        TaintSet = self.GetTaintedParas (LiveObj)
        print ("TaintSet = ", TaintSet)
        self.CallCtx = Context (Func, TaintSet)
        return
    
    def PushCtx (self):
        if self.CallCtx == None:
            return
        self.CtxStack.append (self.CallCtx)
        self.CurCtx = self.CallCtx
        print ("-------------------------> Push Context: ", self.CurCtx.Func)
        return   
    
    def PopCtx (self):
        PopCtx = self.CtxStack[-1]
        self.CtxStack.pop ()
        self.CurCtx = self.CtxStack[-1]
        print ("-------------------------> Pop Context: ", PopCtx.Func)
        return

    def Propogate (self, LiveObj):
        if LiveObj.Def == None:
            return
        Uses = LiveObj.Uses
        for use in Uses:
            if self.CurCtx.IsTaint (use):
                self.CurCtx.InsertLexicon (LiveObj.Def)
                return
        return

    def Real2FormalParas (self, LiveObj):
        Index = 0
        Uses  = LiveObj.Uses
        for use in Uses:
            if Index in self.CurCtx.TaintInpara:
                self.CurCtx.InsertLexicon (use)
            Index += 1
        return

    def GetEventType (self, Event, LiveObj):
        if Event == "line" and LiveObj.Callee != None:
            if LiveObj.Callee not in self.Buildins:                
                self.SetCallCtx (LiveObj.Callee, LiveObj)
            return EVENT_CALL
        if Event == "call" and LiveObj.Callee != None:
            self.PushCtx ()
            self.Real2FormalParas (LiveObj)
            return EVENT_FENTRY
        if LiveObj.Ret != False:
            self.PopCtx ()
            return EVENT_RET

        self.Propogate (LiveObj)
        return EVENT_NR

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
        #print(LineNo, Event, Code.co_name, end=" => ")
        LiveObj = self.Analyzer.HandleEvent (ModulePath, Frame, Event, LineNo)
        if LiveObj == None:
            return 
        
        #LiveObj.View ()
        EventTy = self.GetEventType (Event, LiveObj)       
        FuncId = self.Analyzer.GetFuncId (Code.co_name)
        EventId = PyEventTy (FuncId, LineNo, EventTy, 0)
        print ("---> %lx" %EventId)

        return self.Tracing



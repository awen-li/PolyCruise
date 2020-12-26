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

        # init main ctx
        self.CurCtx = Context ("main", [])
        self.CtxStack.append (self.CurCtx)

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
        for use in Uses:
            if self.CurCtx.IsTaint (use):
                TaintSet.append (use)
        return TaintSet

    def PushCtx (self, Func, LiveObj):
        TaintSet = self.GetTaintedParas (LiveObj)
        Ctx = Context (Func, TaintSet)
        self.CtxStack.append (Ctx)
        self.CurCtx = Ctx
        return

    def PopCtx (self):
        self.CtxStack.pop ()
        self.CurCtx = self.CtxStack[-1]
        return

    def GetEventType (self, FuncName, LiveObj):
        if FuncName != self.CurCtx.Func:
            self.PushCtx (FuncName, LiveObj)
            return EVENT_FENTRY
        if LiveObj.Callee != None:
            return EVENT_CALL
        if LiveObj.Ret != False:
            return EVENT_RET
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
        print(LineNo, Event, Code.co_name, end=" => ")
        LiveObj = self.Analyzer.HandleEvent (ModulePath, Frame, Event, LineNo)
        if LiveObj == None:
            return 
        
        #LiveObj.View ()
        EventTy = self.GetEventType (Code.co_name, LiveObj)       
        FuncId = self.Analyzer.GetFuncId (Code.co_name)
        EventId = PyEventTy (FuncId, LineNo, EventTy, 0)
        print ("---> %lx" %EventId)

        return self.Tracing



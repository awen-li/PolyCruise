#!/usr/bin/python

import os
import sys
import inspect
from PyTrace import *
from .Criterion import Criterion
from .Analyzer import Analyzer

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
    def __init__(self, CurFunc, TaintInPara, Ret=None):
        self.Func = CurFunc
        self.TaintLexical = {}
        self.TaintInpara = TaintInPara
        self.TaintOutpara = []
        self.Ret = Ret
        self.CalleeLo = None

    def InsertLexicon (self, Lexicon):
        if Lexicon == None:
            return
        self.TaintLexical [Lexicon] = True
        #print ("self.TaintLexical = ", self.TaintLexical)

    def IsTaint (self, Lexicon):
        Flag = self.TaintLexical.get (Lexicon)
        if Flag == None:
            return False
        else:
            return True

class Inspector:
    def __init__(self, RecordFile, Criten, EntryFunc="main", SrcDir="."):
        print ("----> __init__................")
        self.Analyzer = Analyzer (RecordFile, SrcDir)
        self.Crtn  = Criten
        self.CtxStack = []
        self.GlobalTaintLexical = {}

        self.CacheMsg = None

        # init main ctx
        self.CallCtx = None
        
        FuncDef = self.Analyzer.GetFuncDef (EntryFunc)
        if FuncDef == None:
            self.CurCtx  = Context (EntryFunc, [])
        else:
            TaintBits    = self.Crtn.GetTaintBits (EntryFunc)
            self.CurCtx  = Context (EntryFunc, TaintBits)
            if TaintBits != None:
                for bit in TaintBits:
                    self.CurCtx.InsertLexicon (FuncDef.Paras[bit])
        
        self.CtxStack.append (self.CurCtx)
        print ("-----------> Push Context: ", self.CurCtx.Func, " Taintlex:", self.CurCtx.TaintLexical)

        self.IsTaint = False
        self.Scripts = ["pyinspect.py", "pyinspect.py", "Inspector.py"]

    def __enter__(self):
        print ("----> __enter__................")
        PyTraceInit ()

        #Entry msg
        FuncDef = self.Analyzer.GetFuncDef (self.CurCtx.Func)
        EventId = PyEventTy (FuncDef.Id, 0, EVENT_FENTRY, 0)
        Msg = "{"+ self.CurCtx.Func + "}"
        print ("Python---> %lx %s" %(EventId, Msg))
        PyTrace (EventId, Msg)

        _settrace(self.Tracing)
        return self

    def __exit__(self, *_):
        if self.CacheMsg != None:
            print ("Python---> %lx %s" %(self.CacheEvent, self.CacheMsg))
            PyTrace (self.CacheEvent, self.CacheMsg)
            self.CacheMsg = None
        print ("----> __exit__................")
        _unsettrace()
        PyTraceExit ()

    def GetTaintedParas (self, LiveObj):
        Uses = LiveObj.Uses
        TaintSet = []
        for Index in range (len(Uses)):
            use = Uses[Index]
            if self.CurCtx.IsTaint (use):
                TaintSet.append (Index)
        return TaintSet

    def SetCallCtx (self, LiveObj):
        TaintBits = self.Crtn.GetTaintBits (LiveObj.Callee)
        if TaintBits != None:
            self.CurCtx.InsertLexicon (LiveObj.Def)
            self.IsTaint = True
            print ("****************<> Add source: ", LiveObj.Def, " = ", LiveObj.Callee)

        TaintSet = self.GetTaintedParas (LiveObj)
        if TaintBits != None:
            TaintSet += TaintBits
            TaintSet = list(set(TaintSet))
        print (LiveObj.Callee, " TaintSet = ", TaintSet)
        self.CallCtx = Context (LiveObj.Callee, TaintSet, LiveObj.Def)
        self.CurCtx.CalleeLo = LiveObj
        return
    
    def PushCtx (self):
        if self.CallCtx == None:
            return
        self.CtxStack.append (self.CallCtx)
        self.CurCtx = self.CallCtx
        self.CallCtx = None 
        print ("-------------------------> Push Context: ", self.CurCtx.Func, " ret = ", self.CurCtx.Ret)
        return   
    
    def PopCtx (self):
        PopCtx = self.CtxStack[-1]
        self.CtxStack.pop ()
        self.CurCtx = self.CtxStack[-1]
        print ("-------------------------> Pop Context: ", PopCtx.Func)
        return

    def Propogate (self, LiveObj):
        Uses = LiveObj.Uses
        for use in Uses:
            if self.CurCtx.IsTaint (use):
                self.IsTaint = True
                if LiveObj.Def != None:
                    self.IsTaint = True
                    self.CurCtx.InsertLexicon (LiveObj.Def)
                break
        return

    def Real2FormalParas (self, LiveObj):
        Index = 0
        Uses  = LiveObj.Uses
        for use in Uses:
            if Index in self.CurCtx.TaintInpara:
                self.IsTaint = True
                self.CurCtx.InsertLexicon (use)
            Index += 1
        return

    def Ret2Callsite (self, LiveObj):
        Ret = LiveObj.Uses[0]
        if self.CurCtx.IsTaint (Ret):
            return True, self.CurCtx.Ret
        else:
            return False, None

    def TaintAnalysis (self, Event, LiveObj):
        if Event == "line" and LiveObj.Callee != None:
            if self.Analyzer.GetFuncDef (LiveObj.Callee) != None:                
                self.SetCallCtx (LiveObj)
            else:
                self.Propogate (LiveObj)
            return EVENT_CALL
        if Event == "call" and LiveObj.Callee != None:
            self.PushCtx ()
            self.Real2FormalParas (LiveObj)
            return EVENT_FENTRY
        if LiveObj.Ret != False:
            Ret = None
            if len (LiveObj.Uses) != 0:
                Taint, Ret = self.Ret2Callsite (LiveObj)
                self.IsTaint = Taint
            self.PopCtx ()
            if Ret != None:
                self.CurCtx.InsertLexicon (Ret)
                #print ("****************<> Ret Taint: ", Ret)
            return EVENT_RET

        self.Propogate (LiveObj)
        return EVENT_NR

    def FormatDefUse (self, LiveObj):
        Msg = ""
        if LiveObj.Def != None:
            Msg += LiveObj.Def + ":U="
        
        for use in LiveObj.Uses:
            Msg += str(use) + ":U"
            if use != LiveObj.Uses[-1]:
                Msg += ","
        return Msg

    def IsLiveObjValid (self, LiveObj):
        if LiveObj == None:
            return False

        if LiveObj.Ret == True and self.CurCtx.Func != "main":
            return True

        if LiveObj.Def == None and len (LiveObj.Uses) == 0:
            return False

        return True

    def Tracing(self, Frame, Event, Arg):        
        Code = Frame.f_code
        ModuleName = ""
        ModulePath = ""

        if self.CacheMsg != None:
            print ("Python---> %lx %s" %(self.CacheEvent, self.CacheMsg))
            PyTrace (self.CacheEvent, self.CacheMsg)
            self.CacheMsg = None

        _, ScriptName = os.path.split(Code.co_filename) 
        if ScriptName in self.Scripts:
            return self.Tracing
       
        LineNo  = Frame.f_lineno
        LiveObj = self.Analyzer.HandleEvent (ScriptName, Frame, Event, LineNo)
        if self.IsLiveObjValid (LiveObj) == False:
            return self.Tracing

        print(ScriptName, LineNo, Event, Code.co_name, self.CurCtx.TaintLexical, LiveObj.Class) #, Frame.f_locals
        LiveObj.View ()

        self.IsTaint = False
        EventTy  = self.TaintAnalysis (Frame, Event, LiveObj)
        if self.IsTaint == False:
            return self.Tracing

        FuncDef  = self.Analyzer.GetFuncDef (self.CurCtx.Func)
        if FuncDef == None:
            return self.Tracing
        IsSource = self.Crtn.IsCriterion (LiveObj.Callee)
        EventId  = PyEventTy (FuncDef.Id, LineNo, EventTy, IsSource)     

        Msg = ""
        if EventTy   == EVENT_CALL:
            if self.CallCtx == None:
                self.CacheMsg   = "{" + LiveObj.Callee + "," + self.FormatDefUse (LiveObj) + "}"
                self.CacheEvent = EventId
        elif EventTy == EVENT_FENTRY:
            Msg = "{" + LiveObj.Callee + "}";
        else:
            Msg = "{" + self.FormatDefUse (LiveObj) + "}"
            print ("Python---> %lx %s" %(EventId, Msg))
            PyTrace (EventId, Msg)
            Msg = ""

            if EventTy == EVENT_RET:
                CalleeObj = self.CurCtx.CalleeLo;

                CallCtx = self.CtxStack[-1]
                FuncDef = self.Analyzer.GetFuncDef (CallCtx.Func)
                EventId = PyEventTy (FuncDef.Id, CalleeObj.LineNo, EVENT_CALL, 0)
                Msg = "{" + CalleeObj.Callee + "," + self.FormatDefUse (CalleeObj) + "}"

        if Msg != "":
            print ("Python---> %lx %s" %(EventId, Msg))
            PyTrace (EventId, Msg)
        
        return self.Tracing



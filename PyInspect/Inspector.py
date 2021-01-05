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
    def __init__(self, CurFunc, TaintInPara, Ret=None):
        self.Func = CurFunc
        self.TaintLexical = {}
        self.TaintInpara = TaintInPara
        self.TaintOutpara = []
        self.Ret = Ret
        self.CalleeLo = None

    def InsertLexicon (self, Lexicon):
        self.TaintLexical [Lexicon] = True
        #print ("self.TaintLexical = ", self.TaintLexical)

    def IsTaint (self, Lexicon):
        Flag = self.TaintLexical.get (Lexicon)
        if Flag == None:
            return False
        else:
            return True

class Inspector:
    def __init__(self, RecordFile, EntryFunc):
        print ("----> __init__................")
        self.Analyzer = Analyzer (RecordFile)
        self.Crtn  = Criterion ()
        self.CtxStack = []

        # init main ctx
        self.CallCtx = None
        
        FuncDef      = self.Analyzer.GetFuncDef (EntryFunc)
        TaintBits    = self.Crtn.GetTaintBits (EntryFunc)
        self.CurCtx  = Context (EntryFunc, TaintBits)
        if TaintBits != None:
            for bit in TaintBits:
                self.CurCtx.InsertLexicon (FuncDef.Paras[bit])
        
        self.CtxStack.append (self.CurCtx)
        print ("-----------> Push Context: ", self.CurCtx.Func, " Taintlex:", self.CurCtx.TaintLexical)

        self.IsTaint = False

    def __enter__(self):
        print ("----> __enter__................")
        PyTraceInit ()

        #Entry msg
        FuncDef = self.Analyzer.GetFuncDef (self.CurCtx.Func)
        EventId = PyEventTy (FuncDef.Id, 0, EVENT_FENTRY, 0)
        #PyTrace (EventId, "{"+self.CurCtx.Func+"}")

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

    def SetCallCtx (self, LiveObj):
        if self.Crtn.IsCriterion (LiveObj.Callee):
            self.CurCtx.InsertLexicon (LiveObj.Def)
            self.IsTaint = True
            print ("****************<> Add source: ", LiveObj.Def, " = ", LiveObj.Callee)
            
        TaintSet = self.GetTaintedParas (LiveObj)
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
        if len (LiveObj.Uses) == 0:
            return
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
            #self.IsTaint = True
            return EVENT_CALL
        if Event == "call" and LiveObj.Callee != None:
            self.PushCtx ()
            self.Real2FormalParas (LiveObj)
            return EVENT_FENTRY
        if LiveObj.Ret != False:
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
        #print ("\t", self.CurCtx.TaintLexical)

        self.IsTaint = False
        EventTy  = self.TaintAnalysis (Event, LiveObj)
        #print ("\t Taint flag = ", self.IsTaint)
        if self.IsTaint == True:
            FuncDef  = self.Analyzer.GetFuncDef (Code.co_name)
            IsSource = self.Crtn.IsCriterion (LiveObj.Callee)
            EventId  = PyEventTy (FuncDef.Id, LineNo, EventTy, IsSource)     

            Msg = ""
            if EventTy   == EVENT_CALL:
                if self.CallCtx == None:
                    Msg = "{" + LiveObj.Callee + "," + self.FormatDefUse (LiveObj) + "}"
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



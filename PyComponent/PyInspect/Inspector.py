#!/usr/bin/python

import os
import sys
import inspect
from PyTrace import *
from .Criterion import Criterion
from .Analyzer import Analyzer
from .PyEvent import *


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
        self.Ret = Ret
        self.RetTaint = False
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
        self.Analyzer = Analyzer (RecordFile, SrcDir)
        self.Crtn  = Criten
        self.CtxStack = []
        self.GlobalTaintLexical = {}

        self.CacheMsg = None

        # init main ctx
        self.CallCtx = None
        
        FuncDef = self.Analyzer.GetFuncDef (EntryFunc)
        self.CurCtx  = Context (EntryFunc, [])       
        self.CtxStack.append (self.CurCtx)

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
                if LiveObj.Class != None:
                    TaintSet.append (Index+1)
                else:
                    TaintSet.append (Index)
        return TaintSet

    def SetCallCtx (self, LiveObj):
        TaintSet = self.GetTaintedParas (LiveObj)
        TaintBits= self.Crtn.GetTaintParas (LiveObj.Callee)
        if TaintBits != None:
            TaintSet += TaintBits
            TaintSet = list(set(TaintSet))

        self.CallCtx = Context (LiveObj.Callee, TaintSet, LiveObj.Def)
        self.CurCtx.CalleeLo = LiveObj
        #print ("\t@@@@@", self.CurCtx.Func, TaintSet, " Cache Callee: ", end="")
        #LiveObj.View ()
        return

    def Actual2Formal (self, CallSiteObj):
        TaintedFormal = []
        
        if CallSiteObj == None:
            return TaintedFormal, None
            
        FCalleeDef = self.Analyzer.GetFuncDef (CallSiteObj.Callee)
        if len (FCalleeDef.Paras) == 0:
            return
        FCalleeDef.View ()

        Index = 0
        ParaNum = 0
        for use in CallSiteObj.Uses:
            if use not in self.CurCtx.TaintLexical:
                Index += 1
                continue
            TaintedFormal.append(FCalleeDef.Paras[Index])
            Index += 1  
            
        return TaintedFormal, CallSiteObj.Callee
    
    def PushCtx (self):
        if self.CallCtx == None:
            return

        # actual -> formal
        CallSiteObj = self.CurCtx.CalleeLo
        TaintedFormal, Callee = self.Actual2Formal (CallSiteObj)
        
        self.CtxStack.append (self.CallCtx)
        self.CurCtx = self.CallCtx
        self.CallCtx = None 
        print ("-------------------------> Push Context: ", self.CurCtx.Func, " ret = ", self.CurCtx.Ret, " Formal = ", TaintedFormal)

        for para in TaintedFormal:
            self.CurCtx.InsertLexicon (para)
            self.IsTaint = True
        return   
    
    def PopCtx (self):
        PopCtx = self.CtxStack[-1]
        self.CtxStack.pop ()
        self.CurCtx = self.CtxStack[-1]
        print ("-------------------------> Pop Context: ", PopCtx.Func)

        #trace the call-site
        CallSiteObj = self.CurCtx.CalleeLo
        TaintedFormal, Callee = self.Actual2Formal (CallSiteObj)
        if len (TaintedFormal) == 0:
            return
                
        FCallerDef = self.Analyzer.GetFuncDef (self.CurCtx.Func)
        EventId = PyEventTy (FCallerDef.Id, CallSiteObj.LineNo, EVENT_CALL, 0)

        Msg = "{" + Callee + "("           
        ParaNum = 0
        for use in TaintedFormal:           
            if ParaNum > 0:
                Msg += ","          
            Msg += use + ":U"      
            ParaNum += 1
        Msg += ")," + self.FormatDefUse (CallSiteObj) + "}"
        
        PyTrace (EventId, Msg)
        print ("$$$ Python---> %lx %s" %(EventId, Msg))
        return

    def Propogate (self, LiveObj):
        self.IsSource = self.Crtn.IsCriterion (self.CurCtx.Func, LiveObj.Def)
        if self.IsSource:
            self.IsTaint = True
            self.CurCtx.InsertLexicon (LiveObj.Def)
            return
        
        Uses = LiveObj.Uses
        for use in Uses:
            if self.CurCtx.IsTaint (use):
                self.IsTaint = True
                if LiveObj.Def != None:
                    self.IsTaint = True
                    self.CurCtx.InsertLexicon (LiveObj.Def)
                break
        return
        

    def Ret2Callsite (self, LiveObj):
        Ret = LiveObj.Uses[0]
        if self.CurCtx.IsTaint (Ret):
            return True, self.CurCtx.Ret
        else:
            return False, None

    def TaintAnalysis (self, Event, LiveObj):
        self.IsSource = False
        self.IsTaint  = False
        if Event == "line" and LiveObj.Callee != None:
            if self.Analyzer.GetFuncDef (LiveObj.Callee) != None:                
                self.SetCallCtx (LiveObj)
            else:
                IsCrn = self.Crtn.IsCriterion (LiveObj.Callee, None)
                if IsCrn != False:
                    self.CurCtx.InsertLexicon (LiveObj.Def)
                    self.IsTaint = True
                    print ("****************<> Add source: ", LiveObj.Def, " = ", LiveObj.Callee)
            
                self.Propogate (LiveObj)
            return EVENT_CALL
        
        if Event == "call" and LiveObj.Callee != None:
            self.PushCtx ()
            return EVENT_FENTRY
        
        if LiveObj.Ret == LiveObject.RET_VALUE:
            Ret = None
            if len (LiveObj.Uses) != 0:
                Taint, Ret = self.Ret2Callsite (LiveObj)
                self.IsTaint = Taint
                self.CurCtx.RetTaint = Taint
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

        if LiveObj.Ret == LiveObject.RET_CALLER and self.CurCtx.Func != "main":
            return True

        if LiveObj.Def == None and len (LiveObj.Uses) == 0:
            return False

        return True

    def Tracing(self, Frame, Event, Arg):        
        Code = Frame.f_code

        #print ("Tracing -> ", Code.co_filename)

        if self.CacheMsg != None:
            print ("Python---> %lx %s" %(self.CacheEvent, self.CacheMsg))
            PyTrace (self.CacheEvent, self.CacheMsg)
            self.CacheMsg = None

        _, ScriptName = os.path.split(Code.co_filename) 
        if ScriptName in self.Scripts:
            return self.Tracing
       
        LineNo  = Frame.f_lineno
        LiveObj = self.Analyzer.HandleEvent (Code.co_filename, Frame, Event, LineNo)
        if self.IsLiveObjValid (LiveObj) == False:
            return self.Tracing

        # return to caller
        if LiveObj.Ret == LiveObject.RET_CALLER:
            Ret = None
            if self.CurCtx.RetTaint == True:
                Ret = self.CurCtx.Ret          
            self.PopCtx ()
            if Ret != None:
                self.CurCtx.InsertLexicon (Ret)
                print ("****************<> Ret Taint: ", Ret)      
            return self.Tracing
        #print(ScriptName, LineNo, Event, Code.co_name, self.CurCtx.TaintLexical, LiveObj.Class) #, Frame.f_locals
        #LiveObj.View ()
 
        EventTy  = self.TaintAnalysis (Event, LiveObj)
        if self.IsTaint == False:
            return self.Tracing

        FuncDef  = self.Analyzer.GetFuncDef (self.CurCtx.Func)
        if FuncDef == None:
            return self.Tracing
        
        EventId  = PyEventTy (FuncDef.Id, LineNo, EventTy, self.IsSource)     

        Msg = ""
        if EventTy   == EVENT_CALL:
            if self.CallCtx == None:
                self.CacheMsg   = "{" + LiveObj.Callee + "," + self.FormatDefUse (LiveObj) + "}"
                self.CacheEvent = EventId
        elif EventTy == EVENT_FENTRY:
            Msg = "{" + LiveObj.Callee + "}";
        else:
            Msg = "{" + self.FormatDefUse (LiveObj) + "}"                

        if Msg != "":
            print ("Python---> %lx %s" %(EventId, Msg))
            PyTrace (EventId, Msg)
        
        return self.Tracing



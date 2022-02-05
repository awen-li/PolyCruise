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
    def __init__(self, CurFunc, FuncDef, TaintInPara, Ret=None):
        self.Func = CurFunc
        self.FuncDef = FuncDef
        self.TaintSymbs  = {}
        self.TaintInpara = TaintInPara
        self.Ret = Ret
        self.RetTaint = False
        self.CalleeLo = None

    def InsertSymb (self, Symb):
        if Symb == None:
            return
        self.TaintSymbs [Symb] = True
        #print ("self.TaintSymbs = ", self.TaintSymbs)

    def IsTaint (self, Symb):
        Flag = self.TaintSymbs.get (Symb)
        if Flag == None:
            return False
        else:
            return True

class Inspector:
    def __init__(self, RecordFile, Criten, PyMap="Pymap.ini", EntryFunc="pyinspect", SrcDir="."):
        self.Analyzer = Analyzer (RecordFile, SrcDir, PyMap)
        self.Crtn  = Criten
        self.CtxStack = []
        self.GlobalTaintSymbs = {}

        self.CacheMsg = None

        # init main ctx
        self.CallCtx = None
        self.CoName  = None
        
        FuncDef = self.Analyzer.GetFuncDef (EntryFunc)
        assert FuncDef != None
        
        self.CurCtx  = Context (EntryFunc, FuncDef, [])       
        self.CtxStack.append (self.CurCtx)

        self.IsTaint = False
        self.Scripts = ["pyinspect.py", "pyinspect.py", "Inspector.py"]

        self.TracedStmts = 0
        self.SetUp = False
        if os.environ.get ("case_name") == None:
            self.IsSingleCase = False
        else:
            self.IsSingleCase = True

        self.Debug = False
        if os.environ.get ("DEBUG") != None:
            self.Debug = True

    def __enter__(self):
        print ("----> __enter__................IsSingleCase->", self.IsSingleCase)
        PyTraceInit ()

        #Entry msg
        FuncDef = self.Analyzer.GetFuncDef (self.CurCtx.Func)
        EventId = PyEventTy (FuncDef.Id, 0, EVENT_FENTRY, 0)
        Msg = "{"+ self.CurCtx.Func + "}"
        #print ("Python---> %lx %s" %(EventId, Msg))
        PyTrace (EventId, Msg)

        if os.environ.get ("FULL_INSTRUMENTATION") != None:
            _settrace(self.TracingFull)
            Msg = "[Python]: " + self.CurCtx.Func
            PyTrace (0, Msg)
        else:
            _settrace(self.Tracing)
        return self

    def __exit__(self, *_):
        if self.CacheMsg != None:
            print ("Python---> %lx %s" %(self.CacheEvent, self.CacheMsg))
            PyTrace (self.CacheEvent, self.CacheMsg)
            self.CacheMsg = None
        print ("----> __exit__................, TracedStmts = ", self.TracedStmts)
        _unsettrace()
        PyTraceExit ()

    def GetTaintedParas (self, LiveObj):
        Uses = LiveObj.Uses
        TaintSet = []
        for Index in range (len(Uses)):
            use = Uses[Index]
            if self.IsInTainted (use):
                if LiveObj.Class != None:
                    TaintSet.append (Index+1)
                else:
                    TaintSet.append (Index)
        return TaintSet

    def IsGlobal (self, Symb):
        if not isinstance (Symb, str):
            return False
        
        if Symb.find('.') != -1:
            return True
        return False

    def IsGlobalTainted (self, Symb):
        Exist = self.GlobalTaintSymbs.get (Symb)
        if Exist != None:
            return True
        return False

    def IsInTainted (self, Symb):
        #print ("@@@@@@@@@ IsInTainted -> ",  Symb)
        #1. local
        Taint = self.CurCtx.IsTaint (Symb)
        if Taint == True:
            return True
        #2. global field-sensitive
        Taint = self.IsGlobalTainted (Symb)
        if Taint == True:
            return True      
        #3. global field-insensitive
        if not isinstance (Symb, str):
            return False
        Obj = Symb.split (".")[0]
        Taint = self.IsGlobalTainted (Symb)
        if Taint == True:
            return True
        return False

    def InsertSymb (self, Symb, Global=False):
        Cls = self.IsGlobal (Symb) or Global
        if Cls == False:
            self.CurCtx.InsertSymb (Symb)
        else:
            self.GlobalTaintSymbs[Symb] = True
        return
        
    def SetCallCtx (self, LiveObj, FuncDef):
        TaintSet = self.GetTaintedParas (LiveObj)
        assert FuncDef != None
        self.CallCtx = Context (LiveObj.Callee, FuncDef, TaintSet, LiveObj.Def)
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
            return TaintedFormal, CallSiteObj.Callee
        #FCalleeDef.View ()

        # query from Criterion       
        DefCrit = self.Crtn.GetSrcArgs (CallSiteObj.Callee)
        if not DefCrit is None:
            #print ("Get into source : ", CallSiteObj.Callee, ", args -> ", DefCrit)
            pass

        Index = 0
        ParaNum = 0
        for use in CallSiteObj.Uses:
            if DefCrit == None:
                if not self.IsInTainted(use):
                    Index += 1
                    continue
            else:
                if DefCrit[Index] != '1':
                    Index += 1
                    continue
            if Index >= len (FCalleeDef.Paras):
                break
            TaintedFormal.append(FCalleeDef.Paras[Index])
            Index += 1  

        if self.Debug == True:
            print ("===> CallSiteObj: class= ", CallSiteObj.Class, ", TaintedFormal = ", TaintedFormal)
        return TaintedFormal, CallSiteObj.Callee
    
    def PushCtx (self, CalleeFunc):
        if self.CallCtx == None:
            FuncDef = self.Analyzer.GetFuncDef (CalleeFunc)
            if FuncDef != None:
                self.CurCtx = Context (CalleeFunc, FuncDef, [])
                self.CtxStack.append (self.CurCtx)
            return

        # actual -> formal
        CallSiteObj = self.CurCtx.CalleeLo
        if CallSiteObj != None and CalleeFunc.find(CallSiteObj.Callee) == -1 and CallSiteObj.Callee.find (CalleeFunc) == -1:
            #print ("@@ Warning: Callee-<", CallSiteObj.Callee, " -> ", CalleeFunc, "> not consistent!!!!")
            return
        TaintedFormal, _ = self.Actual2Formal (CallSiteObj)
        
        self.CtxStack.append (self.CallCtx)
        self.CurCtx = self.CallCtx
        self.CallCtx = None
        if self.Debug == True:
            print ("-------------------------> Push Context: ", self.CurCtx.Func, " ret = ", self.CurCtx.Ret, " Formal = ", TaintedFormal)

        for para in TaintedFormal:
            self.InsertSymb (para)
        self.IsTaint = True
        return   
    
    def PopCtx (self):
        if len (self.CtxStack) <= 1:
            return
        PopCtx = self.CtxStack[-1]
        self.CtxStack.pop ()
        self.CurCtx = self.CtxStack[-1]
        if self.Debug == True:
            print ("-------------------------> Pop Context: ", PopCtx.Func)

        #trace the call-site
        CallSiteObj = self.CurCtx.CalleeLo
        if CallSiteObj == None:
            return

        if self.Debug == True:
            print ("CallSiteObj.Callee poped: ", CallSiteObj.Callee)
        
        TaintedFormal, Callee = self.Actual2Formal (CallSiteObj)
        IsSource = self.Crtn.IsCriterion (Callee)
        if IsSource != False:
            self.InsertSymb (CallSiteObj.Def)
        else:
            if self.Crtn.GetSrcArgs (Callee) != None:
                IsSource = True
                if self.Debug == True:
                    print ("****************<> Add source with Args: ", Callee)
                    
        FCallerDef = self.CurCtx.FuncDef
        EventId = PyEventTy (FCallerDef.Id, CallSiteObj.LineNo, EVENT_CALL, IsSource)

        Msg = "{" + Callee + "("           
        ParaNum = 0
        for use in TaintedFormal:           
            if ParaNum > 0:
                Msg += ","          
            Msg += use + self.GetSymbType (use)      
            ParaNum += 1
        Msg += ")," 

        if len (TaintedFormal) != 0: 
            Msg += self.FormatDefUse (CallSiteObj)
        else:
            if CallSiteObj.Def != None:
                Msg += CallSiteObj.Def + self.GetSymbType (CallSiteObj.Def) + "="
        
        Msg += "}"
        
        PyTrace (EventId, Msg)
        if self.Debug == True:
            print ("$$$ Python---> %lx %s" %(EventId, Msg))
        return

    def Propogate (self, LiveObj):
        IsSource = self.Crtn.IsCriterion (self.CurCtx.Func)
        if IsSource:
            self.IsTaint = True
            self.IsSource = True
            self.InsertSymb (LiveObj.Def)
            return
        
        Uses = LiveObj.Uses
        for use in Uses:
            if self.IsInTainted (use):
                self.IsTaint = True
                if LiveObj.Def != None:
                    self.InsertSymb (LiveObj.Def)
                if LiveObj.UseClf != None:
                    self.InsertSymb (LiveObj.UseClf, True)
                break
        return
        

    def Ret2Callsite (self, LiveObj):
        Ret = LiveObj.Uses[0]
        if self.IsInTainted (Ret):
            return True, self.CurCtx.Ret
        else:
            return False, None

    def SdaAnalysis (self, Event, LiveObj):
        self.IsSource = False
        self.IsTaint  = False
        if Event == "line":
            if LiveObj.Callee != None:
                FuncDef = self.Analyzer.GetFuncDef (LiveObj.Callee)
                if FuncDef != None:
                    self.SetCallCtx (LiveObj, FuncDef)
                    if self.Debug == True:
                        print ("---> FuncDef = LineCall: ", LiveObj.Callee, ", CallCtx = ", self.CallCtx)
                else:
                    if self.Debug == True:
                        print ("---> FuncDef = LineCall: ", LiveObj.Callee, ", CallCtx = None")
                    self.CallCtx = None
                    IsCrn = self.Crtn.IsCriterion (LiveObj.Callee)
                    if IsCrn != False:
                        self.InsertSymb (LiveObj.Def)
                        self.IsTaint = True
                        self.IsSource = True
                        if self.Debug == True:
                            print ("****************<> Add source: ", LiveObj.Def, " = ", LiveObj.Callee)  
                    self.Propogate (LiveObj)
                return EVENT_CALL
            elif LiveObj.Ret == LiveObject.RET_VALUE:
                Ret = None
                if len (LiveObj.Uses) != 0:
                    Taint, Ret = self.Ret2Callsite (LiveObj)
                    self.IsTaint = Taint
                    self.CurCtx.RetTaint = Taint
                return EVENT_RET
            else:
                # in some cases of runtime, no explicit 'call' statement, mannually change the context...
                if self.CallCtx != None and self.CurCtx.CalleeLo != None and self.CoName == self.CurCtx.CalleeLo.Callee and self.CoName != self.CurCtx.Func:
                    #print ("implicit 'call' statement process....", self.CoName)
                    # 1. first push the context to stack
                    self.PushCtx (self.CoName)
                    # 2. trace a entry event
                    FuncDef = self.CurCtx.FuncDef
                    EventId = PyEventTy (FuncDef.Id, 0, EVENT_FENTRY, self.IsSource)
                    PyTrace (EventId, '{'+self.CoName+'}')
                    # 3. trace current event
                    # -> go to other statement tracing
        elif Event == "call" and LiveObj.Callee != None:
            if self.CurCtx.CalleeLo != None:
                self.PushCtx (LiveObj.Callee)
                return EVENT_FENTRY
            else:
                FuncDef = self.Analyzer.GetFuncDef (LiveObj.Callee)
                EventId = PyEventTy (FuncDef.Id, 0, EVENT_FENTRY, self.IsSource)
                PyTrace (EventId, '{'+LiveObj.Callee+'}')
                self.CurCtx.FuncDef = FuncDef

        # other statement tracing
        self.Propogate (LiveObj)
        if LiveObj.Br == True:
            return EVENT_BR
        
        return EVENT_NR

    def GetSymbType (self, Symb):
        if self.IsGlobalTainted (Symb) == True:
            return ":G"
        else:
            return ":U"

    def FormatDefUse (self, LiveObj):
        Msg = ""
        if LiveObj.Def != None:
            Msg += LiveObj.Def + self.GetSymbType (LiveObj.Def) + "="
        
        for use in LiveObj.Uses:
            Msg += str(use) + self.GetSymbType (use)
            if use != LiveObj.Uses[-1]:
                Msg += ","
        return Msg

    def IsLiveObjValid (self, LiveObj, Event):
        if LiveObj == None:
            return False

        if Event == "call":
            return True

        if LiveObj.Ret == LiveObject.RET_CALLER and self.CurCtx.Func != "main":
            return True

        if LiveObj.Def == None and len (LiveObj.Uses) == 0:
            return False

        return True

    def Tracing(self, Frame, Event, Arg):
        if self.IsSingleCase and self.SetUp == False:
            Step = os.environ.get ("case_step")
            if Step == "setup":
                self.SetUp = True
            else:
                return self.Tracing
        
        self.TracedStmts += 1
        Code = Frame.f_code
        self.CoName = Code.co_name

        if self.CacheMsg != None:
            if self.Debug == True:
                print ("Python---> %lx %s" %(self.CacheEvent, self.CacheMsg))
            PyTrace (self.CacheEvent, self.CacheMsg)
            self.CacheMsg = None

        _, ScriptName = os.path.split(Code.co_filename) 
        if ScriptName in self.Scripts:
            PyTraceStop ()
            return self.Tracing
            
        #print (Code.co_filename, ":", Frame.f_lineno, ":", Event)
            
        LineNo  = Frame.f_lineno
        LiveObj = self.Analyzer.HandleEvent (Code.co_filename, Frame, Event, LineNo)
        if self.IsLiveObjValid (LiveObj, Event) == False:
            return self.Tracing

        if self.Debug == True:
            print("@@@@@ ->", ScriptName, LineNo, Event, Code.co_name, "<Local>", self.CurCtx.TaintSymbs, " <Global>", self.GlobalTaintSymbs, ", CurCtx=", self.CurCtx.Func)
            LiveObj.View ()

        # return to caller
        if LiveObj.Ret == LiveObject.RET_CALLER:
            Ret = None
            if self.CurCtx.RetTaint == True:
                Ret = self.CurCtx.Ret          
            self.PopCtx ()
            if Ret != None:
                self.InsertSymb (Ret)
                if self.Debug == True:
                    print ("****************<> Ret Taint: ", Ret)      
            return self.Tracing
 
        EventTy  = self.SdaAnalysis (Event, LiveObj)
        if self.IsTaint == False:
            return self.Tracing

        FuncDef = self.CurCtx.FuncDef
        if FuncDef == None:
            #print ("Warning ---> FuncDef == None where function = ", self.CurCtx.Func)
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
            if self.Debug == True:
                print ("Python---> %lx %s" %(EventId, Msg))
            PyTrace (EventId, Msg)
        
        return self.Tracing


    def TracingFull(self, Frame, Event, Arg):
        if self.IsSingleCase and self.SetUp == False:
            Step = os.environ.get ("case_step")
            if Step == "setup":
                self.SetUp = True
            else:
                return self.TracingFull
        
        self.TracedStmts += 1
        Code = Frame.f_code
        self.CoName = Code.co_name

        _, ScriptName = os.path.split(Code.co_filename) 
        if ScriptName in self.Scripts:
            return self.TracingFull
            
        LineNo  = Frame.f_lineno
        LiveObj = self.Analyzer.HandleEvent (Code.co_filename, Frame, Event, LineNo)
        if self.IsLiveObjValid (LiveObj, Event) == False:
            return self.TracingFull

        if LiveObj.Callee == None or LiveObj.Callee in ['str']:
            return self.TracingFull        

        Msg = "[Python]:" + Event + " = " + ScriptName + ": " +  str(LineNo) + " : " + Code.co_name + "-> " + str(LiveObj.Callee)
        PyTrace (0, Msg)
        return self.TracingFull


#!/usr/bin/python

import sys
import abc
import ast
from ast import *

class LiveObject ():
    RET_CALLER = 2
    RET_VALUE  = 1
    RET_NONE   = 0
    
    def __init__(self):
        self.Def = None
        self.Uses = []
        self.UseClf = None
        self.Callee = None
        self.Ret = LiveObject.RET_NONE
        self.LineNo = 0
        self.Class = None
        self.Br = False

    def SetBr (self, Br):
        self.Br = Br
        
    def SetDef (self, Def):
        self.Def = Def

    def SetLineNo (self, LineNo):
        self.LineNo = LineNo

    def SetUse (self, Use, Clf=None):
        if len (self.Uses) < 3:
            self.Uses.append (Use)
        if self.UseClf == None:
            self.UseClf = Clf

    def SetCallee (self, CallFunc, Class=None):
        self.Callee = CallFunc
        self.Class  = Class

    def SetRet (self, RetFlg = 0):
        self.Ret = RetFlg

    def View (self):      
        if self.Callee != None:
            print ("\t==>[",  self.LineNo, "]Def: ", self.Def, " Use: ", self.Uses, "(", self.UseClf, ")  Ret: ", self.Ret, " Call: ", self.Callee)
        else:
            print ("\t==>[",  self.LineNo, "]Def: ", self.Def, " Use: ", self.Uses, "(", self.UseClf, ")  Ret: ", self.Ret)

class PyEvent(metaclass=abc.ABCMeta):
    def __init__(self, Frame, Event, Statement, Stmt2FuncDef=None):
        self.Frame = Frame
        self.Event = Event
        self.Statement = Statement
        self.Stmt2FuncDef = Stmt2FuncDef
        self.LiveObj = LiveObject ()
  
    @abc.abstractmethod
    def GetDefUse (self):
        pass

    def SetRealDef (self, Def):
        Obj = self.Self2Obj (Def)
        if not isinstance(Obj, (int, str, list, dict, bool, tuple, set)):
            Type = type (Obj)
            self.LiveObj.SetDef (Type.__name__)
        else:
            self.LiveObj.SetDef (Def)

    def SetRealUse (self, Use):
        Obj = self.Self2Obj (Use)
        #print (Use, " ----type-> ", type (Obj))
        if not isinstance(Obj, (int, str, list, dict, bool, tuple, set)):
            Type = type (Obj)
            self.LiveObj.SetUse (Use, Type.__name__)
        else:
            self.LiveObj.SetUse (Use)

    def GetClassType (self, Val):
        Obj = self.Self2Obj (Val)
        Type = type (Obj)
        return Type.__name__

    def Self2Obj (self, Val):
        import numpy as np
        Obj = self.GetLiveObject(Val)
        if type(Obj) is np.ndarray or Obj is None:
            return Val
        return Obj

    def GetLiveObject(self, ValName):
        if isinstance(ValName, Name):
            ValName = ValName.id
        
        Frame = self.Frame
        if ValName in Frame.f_locals:
            return Frame.f_locals[ValName]
        elif ValName in Frame.f_globals:
            return Frame.f_globals[ValName]
        else:
            Builtins = Frame.f_globals['__builtins__']
            if isinstance(Builtins, dict) and (ValName in Builtins):
                return Builtins[ValName]
            elif isinstance(Builtins, Module) and hasattr(Builtins, ValName):
                return getattr(Frame.f_globals['__builtins__'], ValName)
        return None
        #raise ValueError('cannot find the value of {name}'
        #                 ' in frame of {frame}'.format(name=ValName, frame=Frame.f_code.co_name))

    def Default(self, Arg):
        print (ast.dump (self.Statement))
        raise NotImplementedError(self.errorinfo('not implement this kind of '
                                                 'method: {}'.format(dump(self.Statement))))
        
   

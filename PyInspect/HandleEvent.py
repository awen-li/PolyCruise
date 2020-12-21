#!/usr/bin/python

import sys
from ast import *
from PyEvent import PyEvent

class RetEvent (PyEvent):
    def __init__(self, Frame, Event, Statement):
        super(RetEvent, self).__init__(Frame, Event, Statement)

    def GetDefUse (self):
        return


class CallEvent (PyEvent):
    def __init__(self, Frame, Event, Statement):
        super(CallEvent, self).__init__(Frame, Event, Statement)

    def GetDefUse (self):
        Statement = self.Statement
        Callee = Statement.name
        Args   = Statement.args.args

        for arg in Args:
            self.LiveObj.SetUse (arg.arg)
        self.LiveObj.SetCallee (Callee)
        return

class LineEvent (PyEvent):
    def __init__(self, Frame, Event, Statement):
        super(LineEvent, self).__init__(Frame, Event, Statement)

    def GetDefUse (self):
        Statement  = self.Statement
        MethodName = 'LE_' + Statement.__class__.__name__.lower()
        LeMethod   = getattr(self, MethodName, self.Default)
        LeMethod(Statement)
        return
   
    def LE_assign(self, Statement):
        Def = Statement.targets[0].id
        self.LiveObj.SetDef (Def)

        Value = Statement.value
        if isinstance(Value, Name):
            self.LiveObj.SetUse (Value.id)
        elif isinstance(Value, Call):
            self.LE_call (Statement)
        elif isinstance(Value, List):
            for Use in Value.elts:
                self.LiveObj.SetUse (Use)
        elif isinstance(Value, Num):
            self.LiveObj.SetUse (Value.n)
        elif isinstance(Value, BinOp):
            self.LiveObj.SetUse (Value.left.id)
            self.LiveObj.SetUse (Value.right.id)
        elif isinstance(Value, Str):
            pass
        else:
            assert (0), "!!!!!!!!! unknown assignment."

        return

    def LE_expr(self, Statement):
        if not isinstance(Statement.value, Call):
            return None
        self.LE_call (Statement)
        return
        
    def LE_call(self, Statement):
        Callee = Statement.value
        Func   = Callee.func
        if isinstance(Func, Name):
            self.LiveObj.SetCallee (Func.id)
        elif isinstance(Func, Attribute):
            self.LiveObj.SetDef (Func.value.id)
        else:
            print("Unsupport LE_expr -> ", Func.value.id, Func.attr)
            return
        
        Args = Callee.args
        for arg in Args:
            self.LiveObj.SetUse (arg.id)
        return
        

    def LE_return(self, Statement):
        Value = Statement.value
        self.LiveObj.SetRet (True)
        if not isinstance(Value, Name):
            return
        self.LiveObj.SetUse (Value.id)
        
    def LE_functiondef(self, Statement):
        pass
        
    def LE_classdef(self, Statement):
        pass
      
    def LE_if(self, Statement):
        print ("LE_if")
        
    def LE_for(self, Statement):
        Def = Statement.target.elts[1].id
        self.LiveObj.SetDef (Def)
        Use = Statement.iter.args[0].id
        self.LiveObj.SetUse (Use)
        return

        
    def LE_while(self, Statement): 
        print ("LE_if")
        
    def LE_with(self, Statement):
        print ("LE_if")
        
    def LE_excepthandler(self, Statement):
        print ("LE_excepthandler")

    def LE_import(self, Statement):
        print ("LE_import")
        
    def LE_importfrom(self, Statement):
        print ("LE_importfrom")
        
    def LE_assert(self, Statement):
        print ("LE_assert")
        
    def LE_global(self, Statement):
        print ("LE_global")
        
    def LE_delete(self, Statement):
        print ("LE_delete")
        
    def LE_nonlocal(self, Statement):
        print ("LE_nonlocal")
        
    def LE_tryfinally(self, Statement):
        print ("LE_tryfinally")
        
    def LE_tryexcept(self, Statement):
        print ("LE_tryexcept")
        
    def LE_raise(self, Statement):   
        print ("LE_raise")
        
    def LE_print(self, Statement):    
        print ("LE_print")
        
    def LE_exec(self, Statement):    
        print ("LE_exec")
        
    def LE_pass(self, Statement):    
        print ("LE_pass")
        
    def LE_break(self, Statement):   
        print ("LE_break")
        
    def LE_continue(self, Statement): 
        print ("LE_continue")
        
    

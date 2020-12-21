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
            self.Use.append (arg.arg)
        return

class LineEvent (PyEvent):
    def __init__(self, Frame, Event, Statement):
        super(LineEvent, self).__init__(Frame, Event, Statement)

    def GetDefUse (self):
        Statement  = self.Statement
        MethodName = 'LE_' + Statement.__class__.__name__.lower()
        print (MethodName)
        LeMethod   = getattr(self, MethodName, self.Default)
        LeMethod(Statement)
        return
   
    def LE_assign(self, Statement):
        self.Def = Statement.targets[0].id

        Value = Statement.value
        if isinstance(Value, Name):
            self.Use.append(Value.id)
        elif isinstance(Value, Call):
            self.LE_call (Statement)
        elif isinstance(Value, List):
            self.Use = Value.elts
        elif isinstance(Value, Num):
            self.Use = Value.n
        elif isinstance(Value, BinOp):
            self.Use.append (Value.left.id)
            self.Use.append (Value.right.id)
        else:
            print ("!!!!!!!!! unknown assignment.")

        print ("LE_assign")
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
            for arg in Callee.args:
                self.Use.append (arg.id)
            return
        else:
            print("2-LE_expr -> ", Func.value.id, Func.attr)

    def LE_return(self, Statement):
        Value = Statement.value
        if not isinstance(Value, Name):
            return
        self.Use.append (Value.id)
        
    def LE_functiondef(self, Statement):
        pass
        
    def LE_classdef(self, Statement):
        pass
      
    def LE_if(self, Statement):
        print ("LE_if")
        
    def LE_for(self, Statement):
        self.Def = Statement.target.elts[1].id
        Use = Statement.iter.args[0].id
        self.Use.append (Use)
        return

        
    def LE_while(self, Statement):     # hard code, handle in the future
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
        
    

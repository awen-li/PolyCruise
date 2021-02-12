#!/usr/bin/python

import sys
import ast
from ast import *
from .PyEvent import *

class RetEvent (PyEvent):
    def __init__(self, Frame, Event, Statement):
        super(RetEvent, self).__init__(Frame, Event, Statement)

    def GetDefUse (self):
        self.LiveObj.SetRet (LiveObject.RET_CALLER)
        return


class CallEvent (PyEvent):
    def __init__(self, Frame, Event, Statement):
        super(CallEvent, self).__init__(Frame, Event, Statement)

    def GetDefUse (self):
        Statement = self.Statement
        if not isinstance (Statement, FunctionDef):
            return
        Callee = Statement.name
        Args   = Statement.args.args

        Class  = None
        for arg in Args:
            ArgVal = arg.arg
            
            if ArgVal == "self":
                Class  = self.Self2Obj (ArgVal)
                Callee = Class + "." + Callee
                self.LiveObj.SetUse (Class)
            else:
                self.LiveObj.SetUse (ArgVal)
        self.LiveObj.SetCallee (Callee, Class)
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


    def SetDef (self, Target, Statement):
        if isinstance(Target, Name):
            Def = Target.id
            self.LiveObj.SetDef (Def)
        elif isinstance(Target, Attribute):
            Def = Target.value.id
            if hasattr (Target, "attr") == True:
               RealDef = self.Self2Obj (Def)
               #print ("@@@@@@@@@@@@@@ Def: ", Def, " ---> ", RealDef)
               Def = RealDef + "." + Target.attr             
            self.LiveObj.SetDef (Def)
        elif isinstance(Target, Subscript):
            Def = Target.value.id
            self.LiveObj.SetDef (Def)
        else:
            print (ast.dump (Statement))
            assert (0), "!!! LE_assign, unsupport type assignment."
        return

    def SetUse (self, Value, Statement):
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
        elif isinstance(Value, Compare):
            self.LiveObj.SetUse (Value.left.id)
            Cmp = Value.comparators[0]
            if not isinstance(Cmp, NameConstant):
                self.LiveObj.SetUse (Cmp.id)
        elif isinstance(Value, Attribute):
            Use = Value.value.id
            if hasattr (Value, "attr") == True:
                RealUse = self.Self2Obj (Use)
                Use = RealUse + "." + Value.attr
            self.LiveObj.SetUse (Use)
        elif isinstance(Value, Dict):
            dValue = Value.values
            if len (dValue) != 0:
                self.LiveObj.SetUse (dValue[0].id)
        elif isinstance(Value, NameConstant):
            pass
        elif isinstance(Value, UnaryOp):
            pass
        elif isinstance(Value, Subscript):
            self.LiveObj.SetUse (Value.value.id)
        elif isinstance(Value, Tuple):
            Elemts = Value.elts
            for elm in Elemts:
                if not isinstance (elm, Name):
                    continue
                self.LiveObj.SetUse (elm.id)
        else:
            print ("!!!!!!!!! unknown assignment. => ", ast.dump (Statement))
            assert (0), "!!!!!!!!! unknown assignment."
        return
   
    def LE_assign(self, Statement):
        Target = Statement.targets[0]
        self.SetDef (Target, Statement)
        
        Value = Statement.value
        self.SetUse (Value, Statement);
        return

    
    def LE_augassign(self, Statement):
        Target = Statement.target
        self.SetDef (Target, Statement)
        self.LiveObj.SetUse (self.LiveObj.Def)
        
        Value = Statement.value
        self.SetUse (Value, Statement);
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
            #self.LiveObj.SetDef (Func.value.id)
            Class  = self.Self2Obj (Func.value.id)
            CallFunc = Class + "." + Func.attr
            self.LiveObj.SetCallee (CallFunc, Class)
            #oo, add the object as the first parameter
            self.LiveObj.SetUse (Class)
        else:
            print("Unsupport LE_expr -> ", Func.value.id, Func.attr)
            return
        
        Args = Callee.args
        for arg in Args:
            if isinstance(arg, NameConstant):
                continue
            if isinstance (arg, Starred):
                arg = arg.value
            self.LiveObj.SetUse (arg.id)
        return
        

    def LE_return(self, Statement):
        Value = Statement.value
        self.LiveObj.SetRet (LiveObject.RET_VALUE)
        if not isinstance(Value, Name):
            return
        self.LiveObj.SetUse (Value.id)
        
    def LE_functiondef(self, Statement):
        pass

    def LE_asyncfunctiondef(self, Statement):
        pass
        
    def LE_classdef(self, Statement):
        pass
      
    def LE_if(self, Statement):
        Test = Statement.test
        if isinstance(Test, Name):
            self.LiveObj.SetUse (Test.id)
            self.LiveObj.SetBr (True)
        else:
            print ("!!!!!!!!! unknown type in IF. => ", ast.dump (Statement))
            assert (0), "!!!!!!!!! unsupport tyep in IF."
        
    def LE_for(self, Statement):
        Def = Statement.target.elts[1].id
        self.LiveObj.SetDef (Def)
        Use = Statement.iter.args[0].id
        self.LiveObj.SetUse (Use)
        return

        
    def LE_while(self, Statement): 
        #print ("while. => ", ast.dump (Statement))
        pass
        
    def LE_with(self, Statement):
        print ("while. => ", ast.dump (Statement))
        pass
        
    def LE_excepthandler(self, Statement):
        pass

    def LE_import(self, Statement):
        pass
        
    def LE_importfrom(self, Statement):
        pass
        
    def LE_assert(self, Statement):
        pass
        
    def LE_global(self, Statement):
        print ("global. => ", ast.dump (Statement))
        pass
        
    def LE_delete(self, Statement):
        pass
        
    def LE_nonlocal(self, Statement):
        pass
        
    def LE_tryfinally(self, Statement):
        pass
        
    def LE_tryexcept(self, Statement):
        pass
        
    def LE_raise(self, Statement):   
        pass
        
    def LE_print(self, Statement):    
        pass
        
    def LE_exec(self, Statement):    
        pass
        
    def LE_pass(self, Statement):    
        pass
        
    def LE_break(self, Statement):   
        pass
        
    def LE_continue(self, Statement): 
        pass
        
    

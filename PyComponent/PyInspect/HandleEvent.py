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
    def __init__(self, Frame, Event, Statement, Stmt2FuncDef=None):
        super(CallEvent, self).__init__(Frame, Event, Statement, Stmt2FuncDef)

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
                if Callee == "__init__" and self.Stmt2FuncDef != None:
                    FDef = self.Stmt2FuncDef.get (Statement)
                    if FDef != None:
                        Class  = FDef.Cls
                        Callee = FDef.Name
                else:
                    Class  = self.GetClassType (ArgVal)
                    Callee = Class + "." + Callee
                self.LiveObj.SetUse (Class)
            else:
                self.SetRealUse (ArgVal)
        self.LiveObj.SetCallee (Callee, Class)
        return

class LineEvent (PyEvent):
    def __init__(self, Frame, Event, Statement):
        super(LineEvent, self).__init__(Frame, Event, Statement)

    def GetDefUse (self):
        Statement  = self.Statement
        MethodName = 'LE_' + Statement.__class__.__name__.lower()
        LeMethod   = getattr(self, MethodName, self.Default)
        if MethodName != LeMethod.__name__:
            print ("Get method fail ---> ", MethodName)
        LeMethod(Statement)
        return


    def SetDef (self, Target, Statement):
        if isinstance(Target, Name):
            Def = Target.id
            #self.LiveObj.SetDef (Def)
            self.SetRealDef (Def)
        elif isinstance(Target, Attribute):
            Def = Target.value.id
            if hasattr (Target, "attr") == True:
               Class = self.GetClassType (Def)
               #print ("@@@@@@@@@@@@@@ Def: ", Def, " ---> ", Obj.__name__)
               Def = Class + "." + Target.attr             
            self.LiveObj.SetDef (Def)
        elif isinstance(Target, Subscript):
            Def = Target.value.id
            self.SetRealDef (Def)
        else:
            print (ast.dump (Statement))
            assert (0), "!!! LE_assign, unsupport type assignment."
        return

    def SetUse (self, Value, Statement):
        if isinstance(Value, Name):
            #self.LiveObj.SetUse (Value.id)
            self.SetRealUse (Value.id)
        elif isinstance(Value, Call):
            self.LE_call (Statement)
        elif isinstance(Value, Num):
            self.SetRealUse (Value.n)
        elif isinstance(Value, BinOp):
            self.SetUse (Value.left, Statement)
            self.SetUse (Value.right, Statement)
        elif isinstance(Value, Str):
            pass
        elif isinstance(Value, Compare):
            self.SetUse(Value.left, Statement)
            Cmp = Value.comparators[0]
            if not isinstance(Cmp, NameConstant):
                self.SetUse (Cmp, Statement)
        elif isinstance(Value, Attribute):
            Use = Value.value.id
            self.LiveObj.SetUse (Use)
            if hasattr (Value, "attr") == True:
                Class = self.GetClassType (Use)
                Use = Class + "." + Value.attr
            self.LiveObj.SetUse (Use)
        elif isinstance(Value, NameConstant):
            pass
        elif isinstance(Value, UnaryOp):
            pass
        elif isinstance(Value, Subscript):
            self.SetUse (Value.value, Statement)
        elif isinstance(Value, Bytes):
            self.SetRealUse (Value.s)
        elif isinstance(Value, Dict):
            dValue = Value.values
            if len (dValue) != 0:
                Val = dValue[0]
                if not isinstance(Val, NameConstant):
                    self.SetUse (Val, Statement)
        elif isinstance(Value, List):
            for Use in Value.elts:
                if isinstance(Use, Name):
                    self.SetRealUse (Use.id)
                else:
                    #print ("!!!!!!!!! unknown type. => ", ast.dump (Statement))
                    pass
        elif isinstance(Value, Tuple):
            Elemts = Value.elts
            for elm in Elemts:
                if not isinstance (elm, Name):
                    continue
                self.SetRealUse (elm.id)
        elif isinstance(Value, Set):
            for Use in Value.elts:
                if isinstance(Use, Name):
                    self.SetRealUse (Use.id)
                else:
                    #print ("!!!!!!!!! unknown type. => ", ast.dump (Statement))
                    pass
        elif isinstance (Value, JoinedStr):
            pass
        else:
            print ("!!!!!!!!! unknown assignment. => ", ast.dump (Statement))
            assert (0), "!!!!!!!!! unknown assignment."
        return
   
    def LE_assign(self, Statement):
        Target = Statement.targets[0]
        if isinstance(Target, (List, Tuple)):
            for eml in Target.elts:
                if not isinstance(eml, Starred):
                    self.SetDef (eml, Statement)
                    break
        else:
            self.SetDef (Target, Statement)
        
        Value = Statement.value
        self.SetUse (Value, Statement);
        return

    def LE_annassign(self, Statement):
        Target = Statement.target[0]
        self.SetDef (Target, Statement)
        
        Value = Statement.value
        if Value != None:
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
            #print (ast.dump (Statement))
            AttrValue = Func.value
            Class = None
            CallFunc = Func.attr
            if not isinstance (AttrValue, Bytes):
                Class  = self.GetClassType (AttrValue.id)
                if Class == 'module':
                    Class = None
                else:
                    CallFunc = Class + "." + Func.attr
            self.LiveObj.SetCallee (CallFunc, Class)
            #oo, add the object as the first parameter
            self.LiveObj.SetUse (Class)
        else:
            print("Unsupport LE_expr -> ", Func.value.id, Func.attr)
            return
        
        Args = Callee.args
        for arg in Args:
            if isinstance(arg, NameConstant) or isinstance(arg, Bytes):
                continue
            if isinstance (arg, Name):
                self.SetRealUse (arg.id)
            elif isinstance (arg, Starred):
                arg = arg.value
                self.SetRealUse (arg.id)
            elif isinstance (arg, JoinedStr):
                pass
            else:
                print ("!!!!!!!!! unsupport type in CALL. => ", type(arg), "   -ast-> ", ast.dump (Callee))
                #assert (0), "!!!!!!!!! unsupport tyep in CALL."
            
        return
        

    def LE_return(self, Statement):
        Value = Statement.value
        self.LiveObj.SetRet (LiveObject.RET_VALUE)
        if not isinstance(Value, Name):
            return
        self.SetRealUse (Value.id)
        
    def LE_functiondef(self, Statement):
        pass

    def LE_asyncfunctiondef(self, Statement):
        pass
        
    def LE_classdef(self, Statement):
        pass

    def LE_try(self, Statement):
        pass
      
    def LE_if(self, Statement):
        Test = Statement.test
        if isinstance(Test, Name):
            self.SetRealUse (Test.id)
            self.LiveObj.SetBr (True)
        else:
            print ("!!!!!!!!! unknown type in IF. => ", ast.dump (Statement))
            assert (0), "!!!!!!!!! unsupport tyep in IF."
        
    def LE_for(self, Statement):
        Def = Statement.target.elts[1].id
        self.LiveObj.SetDef (Def)
        Use = Statement.iter.args[0].id
        self.SetRealUse (Use)
        return

        
    def LE_while(self, Statement): 
        #print ("while. => ", ast.dump (Statement))
        pass
        
    def LE_with(self, Statement):
        #print ("LE_with. => ", ast.dump (Statement))
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
        
    

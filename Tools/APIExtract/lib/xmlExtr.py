#!/usr/bin/python
# -*- coding: UTF-8 -*-

import os
import shutil
from xml.dom.minidom import parse
import xml.dom.minidom

class ExternAPI ():
    def __init__(self, FileName, Language):
        self.FileName = FileName
        self.Language = Language
        self.Function = None
        self.RetType  = None
        self.ParaType = []
        self.ExtAPIs  = []

    def View (self):
        print ("%s:%s -- %s %s (%s) : %s"\
               %(self.Language, self.FileName, self.RetType, self.Function, self.ParaType, self.ExtAPIs))


class xmlExtr ():
    def __init__(self, xmlDir="xmls", Output="APIs"):
        self.xmlDir = xmlDir
        self.Output = Output
        if os.path.exists (Output):
            shutil.rmtree (Output)
        os.makedirs(Output)

        self.ExtAPI = {}

    def ProcDecl (self, Decl):
        TypeName = Decl.getElementsByTagName("name")
        
        print ("TypeName: ", TypeName[0], TypeName.length, TypeName[0].childNodes[0].data)

    def ProcType (self, Type):
        TypeName = Type.getElementsByTagName("name")[0]
        return TypeName.childNodes[0].data

    def ProcName (self, Name):
        FuncName = Name.childNodes[0].data
        return FuncName

    def ProcParas (self, ParaList):
        ParaTypes = []
        Paras = ParaList.getElementsByTagName("parameter")
        for para in Paras:
            ParaType = para.getElementsByTagName("name")[0]
            ParaTypes.append (ParaType.childNodes[0].data)
        return ParaTypes

    def ProcBlock (self, Block):
        Callees = []
        CallStmts = Block.getElementsByTagName("call")
        for Call in CallStmts:
            Callee = Call.getElementsByTagName("name")[0]
            Callees.append (Callee.childNodes[0].data)
        return Callees


    def ProcFunc (self, Func, ExtAPI):
        Type = Func.getElementsByTagName("type")[0]
        ExtAPI.RetType = self.ProcType (Type)
        
        Name = Func.getElementsByTagName("name")[1]
        ExtAPI.Function = self.ProcName (Name)
        
        ParaList = Func.getElementsByTagName("parameter_list")[0]
        ExtAPI.ParaType = self.ProcParas (ParaList)

        Block = Func.getElementsByTagName("block")[0]
        ExtAPI.ExtAPIs = self.ProcBlock (Block)

        ExtAPI.View ()


    def ExtrXml (self, xmlFile):
        DOMTree = xml.dom.minidom.parse(xmlFile)
        Unit = DOMTree.documentElement

        FileName = Unit.getAttribute("filename")
        Language = Unit.getAttribute("language")
         
        # iterate all functions
        Functions = Unit.getElementsByTagName("function")
        for Func in Functions:
            ExtAPI = ExternAPI (FileName, Language)
            self.ProcFunc (Func, ExtAPI)

    def Extr (self):
        xmlDirs = os.walk(self.xmlDir) 
        for Path, Dirs, Xmls in xmlDirs:
            for xml in Xmls:
                xmlFile = os.path.join(Path, xml)
                self.ExtrXml (xmlFile)
   

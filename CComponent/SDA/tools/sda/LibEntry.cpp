//===-- LibEntry.cpp - get entry function for libraries -------------------===//
//
// Copyright (C) <2019-2024>  <Wen Li>
//
//                      The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "Fts.h"
#include "Sda.h"
#include<mxml.h>


using namespace llvm;
using namespace std;

static inline VOID TranslateSS (ModuleManage *Mm, set <string> *SStr, set <Source*> *SS)
{
    for (auto It = SStr->begin(); It != SStr->end (); It++)
    {
        string ApiName = *It;
        Source *S = new Source (Mm, "", ApiName, TAINT_RET);
        assert (S != NULL);

        if (!S->Check ())
        {
            delete S;
            continue;
        }
        
        SS->insert (S);
    }
    
    return;
}

/* <criterion> 
        <function>module.getenv</function> 
        <return>True</return> 
        <local>None</local> 
   </criterion> */
VOID LoadCriterion (char *XmlDoc, ModuleManage *Mm, set <Source*> *SS)
{

    FILE *fp = fopen(XmlDoc, "r");
    if (fp == NULL)
    {
        printf ("LoadCriterion: open %s fail.....\r\n", XmlDoc);
        return;
    }    
    mxml_node_t* tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
    fclose(fp);

    DWORD No = 0;
    set <string> SStr;
    mxml_node_t* XmlNode = mxmlFindElement(tree, tree, "criterion", NULL, NULL, MXML_DESCEND);
    while (XmlNode != NULL)
    {     
        mxml_node_t *Func  = mxmlFindElement(XmlNode, tree, "function", NULL, NULL, MXML_DESCEND_FIRST);
        mxml_node_t *Ret   = mxmlFindElement(XmlNode, tree, "return", NULL, NULL, MXML_DESCEND_FIRST);      
        mxml_node_t *Local = mxmlFindElement(XmlNode, tree, "local", NULL, NULL, MXML_DESCEND_FIRST);

        const char *FuncName = mxmlGetText(Func, 0);
        //printf("[%u]function:%s, return:%s, local:%s \r\n", No, FuncName, mxmlGetText(Ret, 0), mxmlGetText(Local, 0));

        XmlNode = mxmlFindElement(XmlNode, tree, "criterion", NULL, NULL, MXML_DESCEND);
        No++;

        SStr.insert (FuncName);
    }

    mxmlDelete(tree);

    TranslateSS (Mm, &SStr, SS);
    printf ("LoadCriterion %s success [%u/%u]\r\n", XmlDoc, (unsigned)SS->size(), No);
    
    return;
}

static inline unsigned Str2Bits (const char *StrBits)
{
    unsigned No = RET_NO;
    unsigned Bits = 0;

    const char *C = StrBits;
    while (*C != 0)
    {
        if (*C != '0')
        {
            SET_TAINTED(Bits, No);
        }
        
        No++;
        C++;
    }
    
    //printf ("StrBits=%s ---> %x \r\n", StrBits, Bits);
    return Bits;
}

/* <function_sds>
    <sds>
        <function>numpy_lapack_lite_s_cmp</function> 
        <in>0110000</in> 
        <out>00000000</out>
    </sds>
   </function_sds> */
VOID LoadFuncSds (char *XmlDoc /* /tmp/difg/function_sds.xml */)
{

    FILE *fp = fopen(XmlDoc, "r");
    if (fp == NULL)
    {
        printf ("LoadFuncSds: open %s fail.....\r\n", XmlDoc);
        return;
    }    
    mxml_node_t* tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
    fclose(fp);

    DWORD No = 0;
    set <string> SStr;
    mxml_node_t* XmlNode = mxmlFindElement(tree, tree, "sds", NULL, NULL, MXML_DESCEND);
    while (XmlNode != NULL)
    {     
        mxml_node_t *Func  = mxmlFindElement(XmlNode, tree, "function", NULL, NULL, MXML_DESCEND_FIRST);
        mxml_node_t *In   = mxmlFindElement(XmlNode, tree, "in", NULL, NULL, MXML_DESCEND_FIRST);      
        mxml_node_t *Out = mxmlFindElement(XmlNode, tree, "out", NULL, NULL, MXML_DESCEND_FIRST);

        const char *FuncName = mxmlGetText(Func, 0);
        const char *InBits   = mxmlGetText(In, 0);
        const char *OutBits  = mxmlGetText(Out, 0);
        //printf("[%u]function:%s, in:%s, out:%s \r\n", No, FuncName, InBits, OutBits);
        ExternalLib::AddFuncSds(mxmlGetText(Func, 0), Str2Bits (InBits), Str2Bits (OutBits));

        XmlNode = mxmlFindElement(XmlNode, tree, "sds", NULL, NULL, MXML_DESCEND);
        No++;

    }

    mxmlDelete(tree);

    printf ("LoadFuncSds %s success [%u]\r\n", XmlDoc, No);
    return;
}



static inline unsigned GetCalledFunc (ModuleManage *Mm, set <Function*> *CalledFunc)
{ 
    Fts  Fts (Mm);
    unsigned FuncNum = 0;
    for (auto It = Mm->func_begin (); It != Mm->func_end (); It++)
    {
        Function *Func  = *It;
        if (Func->isDeclaration() || Func->isIntrinsic())
        {
            continue;
        }

        FuncNum++;
        for (inst_iterator itr = inst_begin(*Func), ite = inst_end(*Func); itr != ite; ++itr) 
        {
            Instruction *Inst = &*itr.getInstructionIterator();
            LLVMInst LI (Inst);
            if (!LI.IsCall ())
            {
                continue;
            }

            Function *Callee = LI.GetCallee ();
            if (Callee == NULL)
            {
                continue;
                FUNC_SET *Fset = Fts.GetCalleeFuncs (&LI);
                if (Fset == NULL)
                {
                    continue;
                }
                
                for (auto Fit = Fset->begin(), End = Fset->end(); Fit != End; Fit++)
                {
                    Callee = *Fit;
                    //errs()<<"Indirect Function: "<<Callee->getName ()<<"\r\n";
                    CalledFunc->insert (Callee);
                }
            }
            else
            {
                CalledFunc->insert (Callee);
            }    
        }
    }
    
    return FuncNum;
}


void GetLibEntry (ModuleManage *Mm, set <Function*> *Entry)
{
    set <Function*> CalledFunc;
    set <Function*> DeclFunc;

    unsigned FuncNum = GetCalledFunc (Mm, &CalledFunc);
    for (auto It = Mm->func_begin (); It != Mm->func_end (); It++)
    {
        Function *Func  = *It;
        if (Func->isDeclaration() || Func->isIntrinsic())
        {
            continue;
        }

        auto ItC = CalledFunc.find (Func);
        if (ItC != CalledFunc.end ())
        {
            continue;
        }

        const char *FuncName = Func->getName().data ();
        if (ExternalLib::IsExistSds(FuncName))
        {
            continue;
        }
       
        Entry->insert (Func);
        //errs()<<"Add entry function: "<<Func->getName ()<<"\r\n";
    }
    printf ("GetLibEntry: %u / %u \r\n", (DWORD)Entry->size (), FuncNum);
    return;
}


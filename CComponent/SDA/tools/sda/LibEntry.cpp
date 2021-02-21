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
        printf("[%u]function:%s, return:%s, local:%s \r\n", No, mxmlGetText(Func, 0), mxmlGetText(Ret, 0), mxmlGetText(Local, 0));

        XmlNode = mxmlFindElement(XmlNode, tree, "criterion", NULL, NULL, MXML_DESCEND);
        No++;

        SStr.insert (string (mxmlGetText(Func, 0)));
    }

    mxmlDelete(tree);

    printf ("LoadCriterion %s success [%u]\r\n", XmlDoc, No);
    TranslateSS (Mm, &SStr, SS);
    
    return;
}


static inline void GetCalledFunc (ModuleManage *Mm, set <Function*> *CalledFunc)
{  
    for (auto It = Mm->func_begin (); It != Mm->func_end (); It++)
    {
        Function *Func  = *It;
        if (Func->isDeclaration() || Func->isIntrinsic())
        {
            continue;
        }

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
            }

            CalledFunc->insert (Callee);
        }
    }
    
    return;
}


void GetLibEntry (ModuleManage *Mm, set <Function*> *Entry)
{
    set <Function*> CalledFunc;
    set <Function*> DeclFunc;

    GetCalledFunc (Mm, &CalledFunc);
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
       
        Entry->insert (Func);
        //errs()<<"Add entry function: "<<Func->getName ()<<"\r\n";
    }

    return;
}


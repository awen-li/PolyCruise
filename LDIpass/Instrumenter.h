//===-- Instrumenter.h - instruction level instrumentation ----------------===//
//
// Copyright (C) <2019-2024>  <Wen Li>
//
//                      The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TRANSFORMS_INSTRUMENTATION_INSTRUMENTER_H
#define LLVM_LIB_TRANSFORMS_INSTRUMENTATION_INSTRUMENTER_H
#include <set>
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Support/BranchProbability.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "HookFunc.h"
#include "LLVMInst.h"
#include "LdaBin.h"


using namespace llvm;
using namespace std;

struct CSTaint
{
    CSTaint ()
    {
        m_InTaintBits  = 0;
        m_OutTaintBits = 0;
    }
    
    set <string> m_Callees;
    unsigned m_InTaintBits;
    unsigned m_OutTaintBits;
};

class Flda
{
private:
    string m_FuncName;
    map <unsigned, CSTaint> m_CsID2Cst;
    set <unsigned> m_TaintInstIDs;

public:
    Flda (string FuncName)
    {
        m_FuncName = FuncName;
    }

    ~Flda ()
    {

    }

    inline CSTaint* AddCst (unsigned Id)
    {
        auto It = m_CsID2Cst.find (Id);
        if (It == m_CsID2Cst.end())
        {
            auto Pit = m_CsID2Cst.insert (make_pair(Id, CSTaint ()));
            assert (Pit.second == true);

            return &(Pit.first->second);
        }
        else
        {
            return &(It->second);
        }
    }

    inline bool IsInstTainted (unsigned Id)
    {
        auto It = m_TaintInstIDs.find (Id);
        if (It == m_TaintInstIDs.end())
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    inline void AddInstID (unsigned Id)
    {
         m_TaintInstIDs.insert (Id);
    }
};

class Instrumenter
{
private:
    Module *m_Module;

    Constant *m_TackFunc;
    Constant *m_InitFunc;
    Constant *m_ExitFunc;

    map <string, Flda> m_Fname2Flda;

public:
    
    Instrumenter(Module *M)
    {
        m_Module = M;


        TraceFunc Trb;
        m_TackFunc = Trb.geHookFunction (M);
        assert (m_TackFunc != NULL);
        m_InitFunc = Trb.geInitFunction (M);
        m_ExitFunc = Trb.geExitFunction (M);
    } 

    void RunInstrm ()
    {
        LoadLdaBin ();
        
        VisitFunction ();
    }
    
private:

    inline Flda* GetFlda (string Fname)
    {
        auto It = m_Fname2Flda.find (Fname);
        if (It == m_Fname2Flda.end())
        {
            return NULL;
        }
        else
        {
            return &(It->second);
        }
    }

    inline Flda* AddFlda (string Fname)
    {
        Flda* Fd = GetFlda (Fname);
        if (Fd == NULL)
        {
            auto Pit = m_Fname2Flda.insert (make_pair(Fname, Flda (Fname)));
            assert (Pit.second == true);

            return &(Pit.first->second);
        }
        else
        {
            return Fd;
        }
    }

    inline void LoadLdaBin ()
    {
        FILE *Bf = fopen ("LdaBin.bin", "rb");
        assert (Bf != NULL);
        
        unsigned FldaNum = 0;
        (void)fread (&FldaNum, sizeof(FldaNum), 1, Bf);
        printf ("FldaNum = %u \r\n", FldaNum);

        for (unsigned Fid = 0; Fid < FldaNum; Fid++)
        {
            FldaBin Fdb = {0};
            (void)fread (&Fdb, sizeof(Fdb), 1, Bf);
            Flda *Fd = AddFlda (Fdb.FuncName);

            printf ("Load Function: %s\r\n", Fdb.FuncName);
            for (unsigned Iid = 0; Iid < Fdb.TaintInstNum; Iid++)
            {
                unsigned Id = 0;
                (void)fread (&Id, sizeof(Id), 1, Bf);
                Fd->AddInstID (Id);
                printf ("\tTaintInst: %u\r\n", Id);
            }        

            for (unsigned Cid = 0; Cid < Fdb.TaintCINum; Cid++)
            {
                CSTaintBin Cstb;
                (void)fread (&Cstb, sizeof(Cstb), 1, Bf);

                CSTaint *Cst = Fd->AddCst (Cstb.InstID);
                Cst->m_InTaintBits  = Cstb.InTaintBits;
                Cst->m_OutTaintBits = Cstb.OutTaintBits;
                
                for (unsigned Fid = 0; Fid < Cstb.CalleeNum; Fid++)
                {
                    char CalleeName[F_NAME_LEN] = {0};
                    (void)fread (CalleeName, sizeof(CalleeName), 1, Bf);
                    Cst->m_Callees.insert (CalleeName);
                }
            }
        }

        fclose (Bf);
    }

    inline void AddTrack (Instruction* inst)
    {
        IRBuilder<> Builder(inst);

        llvm::Type *I32ype = llvm::IntegerType::getInt32Ty(m_Module->getContext());

        Value *vDef  = llvm::ConstantInt::get(I32ype, 0, true);
        Value *vUse1 = llvm::ConstantInt::get(I32ype, 0, true);
        Value *vUse2 = llvm::ConstantInt::get(I32ype, 0, true);

        Builder.CreateCall(m_TackFunc, {vDef, vUse1, vUse2});

        return;
    }

    inline void VisitFunction ()
    {
        for (Module::iterator it = m_Module->begin(), eit = m_Module->end(); it != eit; ++it) 
        {
            Function *F = &*it;  
            if (F->isIntrinsic() || F->isDeclaration ())
            {
                continue;
            }

            Flda *Fd = GetFlda (F->getName ().data());
            if (Fd == NULL)
            {
                continue;
            }
            
            unsigned InstId = 0;
            errs()<<"Process Function: "<<F->getName ()<<"\r\n";
            for (auto ItI = inst_begin(*F), Ed = inst_end(*F); ItI != Ed; ++ItI, InstId++) 
            {
                if (!Fd->IsInstTainted (InstId))
                {
                    continue;
                }

                Instruction *Inst = &*ItI;
                AddTrack (Inst);

                printf ("\t TaintInst: %u \r\n", InstId);
            }

        }

        return;
    }
};



#endif // LLVM_LIB_TRANSFORMS_INSTRUMENTATION_INSTRUMENTER_H

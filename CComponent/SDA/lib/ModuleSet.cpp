//===----- ModuleSet.cpp  Packaging for LLVM modules ---------------------//
//
// Copyright (C) <2019-2024>  <Wen Li>
//

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//===----------------------------------------------------------------------===//

#include <llvm/Support/CommandLine.h>
#include <llvm/IR/LLVMContext.h>		
#include <llvm/Support/SourceMgr.h> 
#include <llvm/Bitcode/BitcodeWriter.h>	
#include <llvm/IRReader/IRReader.h>	
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/IRBuilder.h>
#include "ModuleSet.h"

using namespace std;
using namespace llvm;

VOID FunctionSet::AddFunction (llvm::Function* Func)
{
    m_FuncVector.push_back (Func);
    if (Func->isDeclaration()) 
    {
        m_FunDecls.insert(Func);
    } 
    else 
    {
        m_FunDefs.insert(Func);
    }
        
    return;
}

VOID FunctionSet::GetFuncNameSet(T_StringSet &NameSet, T_FunSet& FuncSet)
{
    T_FunSet::iterator it, end;
    
    for (it = FuncSet.begin(), end = FuncSet.end(); it != end; ++it) 
    {
        Function *fDecl = *it;
        NameSet.insert(fDecl->getName().str());
    }

    return;
}

VOID FunctionSet::GetDefAndDeclFunc (T_StringSet& DefAndDeclNameSet)
{
    T_FunVector::iterator it, end;
    T_StringSet DeclNameSet, DefNameSet;
    
    GetFuncNameSet (DeclNameSet, m_FunDecls);
    GetFuncNameSet (DefNameSet,  m_FunDefs);
            
    // Find the intersectNames
    T_StringSet::iterator declIter, defIter;
    declIter = DeclNameSet.begin();
    defIter  = DefNameSet.begin();
    while (declIter != DeclNameSet.end() && defIter != DefNameSet.end()) 
    { 
        if (*declIter < *defIter) 
        {
            declIter++;
        } 
        else if (*declIter > *defIter)
        {
            defIter++;
        }
        else
        {
            DefAndDeclNameSet.insert(*declIter);
            declIter++;
            defIter++;         
        }
    }

    return;
}

VOID FunctionSet::BuildFuncDeclToDefMap (T_StringSet &DefAndDeclNameSet)
{
    T_FunSet::iterator it,end;
    T_NameToFunDefMap nameToFunDefMap;
    
    for (it = m_FunDefs.begin(), end = m_FunDefs.end(); it != end; ++it) 
    {
        Function *fdef = *it;
        string funName = fdef->getName().str();
        if (DefAndDeclNameSet.find(funName) == DefAndDeclNameSet.end())
        {
            continue;
        }
        
        nameToFunDefMap[funName] = fdef;
    }

    for (it = m_FunDecls.begin(), end = m_FunDecls.end(); it != end; ++it)
    {
        Function *fdecl = *it;
        string funName = fdecl->getName().str();
        if (DefAndDeclNameSet.find(funName) == DefAndDeclNameSet.end())
        {
            continue;
        }
        
        T_NameToFunDefMap::iterator mit = nameToFunDefMap.find(funName);
        if (mit == nameToFunDefMap.end())
        {
            continue;
        }
        
        m_FunDeclToDefMap[fdecl] = mit->second;
    }

    return;
}

VOID FunctionSet::BuildFuncDefToDeclMap (T_StringSet &DefAndDeclNameSet)
{
    T_FunSet::iterator it,end;
    T_NameToFunDeclsMap nameToFunDeclsMap;
    
    for (it = m_FunDecls.begin(), end = m_FunDecls.end(); it != end; ++it)
    {
        Function *fdecl = *it;
        string funName = fdecl->getName().str();
        if (DefAndDeclNameSet.find(funName) == DefAndDeclNameSet.end())
        {
            continue;
        }
        
        T_NameToFunDeclsMap::iterator mit = nameToFunDeclsMap.find(funName);
        if (mit == nameToFunDeclsMap.end()) 
        {
            T_FunSet decls;
            decls.insert(fdecl);
            nameToFunDeclsMap[funName] = decls;
        } 
        else 
        {
            T_FunSet &decls = mit->second;
            decls.insert(fdecl);
        }
    }

    for (it = m_FunDefs.begin(), end = m_FunDefs.end(); it != end; ++it) 
    {
        Function *fdef = *it;
        string funName = fdef->getName().str();
        if (DefAndDeclNameSet.find(funName) == DefAndDeclNameSet.end())
        {
            continue;
        }
        
        T_NameToFunDeclsMap::iterator mit = nameToFunDeclsMap.find(funName);
        if (mit == nameToFunDeclsMap.end())
        {
            continue;
        }
        
        T_FunVector decls;
        for (set<Function*>::iterator sit = mit->second.begin(),
                seit = mit->second.end(); sit != seit; ++sit) 
        {
            decls.push_back(*sit);
        }
        m_FunDefToDeclsMap[fdef] = decls;
    }
    
    return;
}


VOID FunctionSet::BuildFuncMap ()
{   
    T_StringSet DefAndDeclNameSet;

    GetDefAndDeclFunc (DefAndDeclNameSet);

    BuildFuncDeclToDefMap (DefAndDeclNameSet);

    BuildFuncDefToDeclMap (DefAndDeclNameSet);

    return;
}


ModuleSet *ModuleManage::m_ModuleSet = NULL;

ModuleSet::ModuleSet(const vector<string> &ModulePathVec)
{
    m_ModuleNum = 0;
    
    m_LlvmCtx   = NULL;
    m_Modules   = NULL;
    m_EntryFunc = NULL;

    InitFmMap();
    
    loadModules(ModulePathVec);
}

ModuleSet::ModuleSet(llvm::Module &Mod) 
{
    m_ModuleNum = 1;
    m_EntryFunc = NULL;
    
    m_LlvmCtx = &(Mod.getContext());
    m_Modules = new unique_ptr<Module>[m_ModuleNum];
    
    m_Modules[0] = std::unique_ptr<llvm::Module>(&Mod);

    InitFmMap();
    
    InitModuleSet();
}

VOID ModuleSet::InitModuleSet ()
{
    llvm::Function *Func;
    
    for (DWORD Id = 0; Id < m_ModuleNum; ++Id) 
    {
        llvm::Module *M = m_Modules[Id].get();
        DEBUG("Initiate module:%d \r\n", Id);

        for (Module::iterator it = M->begin(), eit = M->end(); it != eit; ++it) 
        {
            Func = &*it;
            if (Func->isIntrinsic())
            {
                //continue;
            }

            if (Func->isDeclaration ())
            {
                LoadModuleOnDemand(Func->getName().data());
            }
            
            m_FuncSet.AddFunction( Func);

            if (IsEntryFunction(Func))
            {
                assert(m_EntryFunc == NULL && "multiple entrys?");
                m_EntryFunc = Func;
            }
        }

        for (Module::global_iterator it = M->global_begin(), eit = M->global_end(); it != eit; ++it) 
        {
            m_GlobalSet.AddGlobalVal(&*it);
        }
    }

    m_FuncSet.BuildFuncMap();   
}

void ModuleSet::loadOneModule(string &ModulePath, DWORD Id)
{
    SMDiagnostic Err;

    m_Modules[Id] = parseIRFile(ModulePath, Err, m_LlvmCtx[0]);
    if (!m_Modules[Id]) 
    {
        printf("load module: %s failed\n",  ModulePath.c_str());
        exit(0);
    }

    m_IdToName[Id] = ModulePath;

    return;
}

VOID ModuleSet::loadModules(const vector<string> &ModulePathVec) 
{
    DWORD Ret;
    DWORD Id = 0;

    m_ModulePathVec = ModulePathVec;
    
    m_ModuleNum = ModulePathVec.size();
    assert (m_ModuleNum != 0);

    m_LlvmCtx = new LLVMContext[1];
    m_Modules = new unique_ptr<Module>[m_ModuleNum+MAX_MODULE_NUM];

    for (vector<string>::const_iterator it = ModulePathVec.begin(), end = ModulePathVec.end();
         it != end; it++)
    {
        string ModulePath = *it;
        loadOneModule (ModulePath, Id);
        Id++;
        printf("---> Load Module:[%-2d/%-2d]\r", Id, m_ModuleNum);
    }
    printf("\n");

    if (llaf::GetParaValue (PARA_PREPROFESS) == "1")
    {
        PreProcess ();
    }
    else
    {
        InitModuleSet();
        printf("---> Load Module: function number = %u\n", m_FuncSet.Size());
    }
    
    return;
}

VOID ModuleSet::PreProcess ()
{
    llvm::Function *Func;

    FILE *f = fopen (FUNC_MODULE, "w");
    if (f == NULL)
    {
        return;
    }

    printf("Start preprocessing....\r\n ");
    for (DWORD Id = 0; Id < m_ModuleNum; ++Id) 
    {
        printf("preprocess module:%-4d\r ", Id);
        llvm::Module *M = m_Modules[Id].get();
        for (Module::iterator it = M->begin(), eit = M->end(); it != eit; ++it) 
        {  
            Func = &*it;
            if (Func->isDeclaration ())
            {
                continue;
            }
            fprintf (f, "%s %s \r\n", Func->getName().data (), m_IdToName[Id].c_str());          
        }
    }
    printf("Finish preprocessing....\r\n ");
    fclose(f);

    return;
}


VOID ModuleSet::InitFmMap()
{
    char FuncName[1024];
    char ModuleName[1024];
    
    FILE *f = fopen (FUNC_MODULE, "r");
    if (f == NULL)
    {
        return;
    }

    while (!feof(f))
    {
        if (fscanf(f, "%s %s", FuncName, ModuleName) < 2)
        {
            break;
        }
        
        m_FuncToModule[FuncName] = ModuleName;
    } 

    return;
}

BOOL ModuleSet::IsModuleExist(std::string ModulePath)
{
    auto it = m_ModulePathVec.begin();
    auto end = m_ModulePathVec.end();
    while (it != end)
    {
        if (it->compare(ModulePath) == 0)
        {
            return AF_TRUE;
        }

        it++;
    }

    return AF_FALSE;
}


string ModuleSet::GetModulePathByFname(string FuncName)
{
    auto it = m_FuncToModule.find(FuncName);
    if (it == m_FuncToModule.end())
    {
        return "";
    } 

    return it->second;
}


VOID ModuleSet::LoadModuleOnDemand (string FuncName)
{
    /* get module path by function name */
    string ModulePath = GetModulePathByFname(FuncName);
    if (ModulePath == "")
    {
        return;
    }

    if (IsModuleExist(ModulePath))
    {
        return;
    }

    printf("LoadModuleOnDemand:[%d]%s \r\n", m_ModuleNum+1, ModulePath.c_str());
    loadOneModule (ModulePath, m_ModuleNum);
    m_ModuleNum++;

    m_ModulePathVec.push_back(ModulePath);
    return;
}


inline bool ModuleSet::IsEntryFunction (const llvm::Function * Func) 
{
    if (Func->getName().str() == "main")  
    {
        return true;
    }
    else
    {
        return false;
    }
}




//===- SoftPara.h - PARAMETERS INPUT  ------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// visit directory.
///
//===----------------------------------------------------------------------===//
#ifndef _SOFTPARA_H
#define _SOFTPARA_H
#include "common/BasicMacro.h"

#define PARA_PREPROFESS     (std::string("preprocess"))
#define PARA_CFG_DUMP       (std::string("cfg_dump"))
#define PARA_CFG_WEIGHT     (std::string("cfg_weight"))
#define PARA_DDG_DUMP       (std::string("ddg_dump"))



class SoftPara
{
private:
    std::map<std::string, std::string> m_ParaToValue;

private:
    VOID InitParaList();

public:
    SoftPara();
    ~SoftPara();
    
    VOID SetParaValue(std::string &strPara, std::string &strValue);
    std::string GetParaValue(std::string &strPara);
};


class ParaSet
{
private:
    static SoftPara *m_SoftPara;

public:
    ParaSet ()
    {
        if (m_SoftPara == NULL)
        {
            m_SoftPara = new SoftPara();
            assert (m_SoftPara != NULL);
        }
    }
    
    ~ParaSet()
    {
    }

    inline VOID Release ()
    {
        if (m_SoftPara != NULL)
        {
            delete m_SoftPara;
            m_SoftPara == NULL;
        }       
    }
    
    inline VOID SetParaValue(std::string &strPara, std::string &strValue)
    {
        m_SoftPara->SetParaValue (strPara, strValue);
        return;
    }
    
    inline std::string GetParaValue(std::string &strPara)
    {
        return m_SoftPara->GetParaValue (strPara);
    }
};


namespace llaf
{
    inline VOID SetParaValue(std::string strPara, std::string strValue)
    {
        ParaSet Ps;
        Ps.SetParaValue (strPara, strValue);
        return;
    }
    
    inline std::string GetParaValue(std::string strPara)
    {
        ParaSet Ps;
        return Ps.GetParaValue (strPara);
    }    
}


#endif 

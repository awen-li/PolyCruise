//===- SoftPara.cpp - software parameter configuration- -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
#include "common/SoftPara.h"

SoftPara* ParaSet::m_SoftPara = NULL;

VOID SoftPara::InitParaList()
{
    m_ParaToValue[PARA_PREPROFESS] = "";
    m_ParaToValue[PARA_CFG_DUMP] = "";
    m_ParaToValue[PARA_CFG_WEIGHT] = "";
    m_ParaToValue[PARA_DDG_DUMP] = "";
}


SoftPara::SoftPara()
{
    InitParaList();
}

SoftPara::~SoftPara()
{
}
    
VOID SoftPara::SetParaValue(std::string &strPara, std::string &strValue)
{
    auto it = m_ParaToValue.find(strPara);
    if (it == m_ParaToValue.end())
    {
        printf("Alert: set %s to %s fail!!!\r\n", strPara.c_str(), strValue.c_str());
        return;
    }

    printf("set %s to %s success!!!\r\n", strPara.c_str(), strValue.c_str());
    m_ParaToValue[strPara] = strValue;

    return;
}

std::string SoftPara::GetParaValue(std::string &strPara)
{
    auto it = m_ParaToValue.find(strPara);
    if (it == m_ParaToValue.end())
    {
        printf("Alert: get %s 's value fail!!! \r\n", strPara.c_str());
        return "";
    }

    return it->second;
}




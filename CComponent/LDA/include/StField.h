//===-- StField.h - Structure field sensitive -----------------------------===//
//
// Copyright (C) <2019-2024>  <Wen Li>
//
//                      The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TRANSFORMS_INSTRUMENTATION_STFIELD_H
#define LLVM_LIB_TRANSFORMS_INSTRUMENTATION_STFIELD_H
#include <utility>
#include <map>
#include <set>

using namespace std;

    
class StField
{
typedef set<unsigned> FIELD_SET;

private:
    map<string, FIELD_SET> m_StFields;
    
public:
    StField ()
    {
    }

    ~StField()
    {
    }

public:
    inline void AddStFields (string StName, unsigned Filed)
    {
        m_StFields[StName].insert (Filed);
    }
	
	inline bool IsActiveFields (string StName, unsigned Field)
    {
        auto Mit = m_StFields.find (StName);
        if (Mit != m_StFields.end ())
        {
            printf ("1 IsActiveFields: %s   --->  %u\r\n", StName.c_str(), Field);
            
            FIELD_SET *Fs = &(Mit->second);
            auto Fit = Fs->find (Field);
            if (Fit != Fs->end ())
            {
                printf ("2 IsActiveFields: %s   --->  %u\r\n", StName.c_str(), Field);
                return true;
            }
            else
            {
                return false;
            }
        }
        
        return true;
	}


    
};


#endif // LLVM_LIB_TRANSFORMS_INSTRUMENTATION_STFIELD_H
//===- Bitmap.h - bit map    --------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//
#ifndef _BITMAP_H_
#define _BITMAP_H_
#include "common/BasicMacro.h"

class Bitmap
{
private:
    CHAR *m_Bitmap;
    DWORD m_BitSize;

private:
    inline VOID ExtednMemory ()
    {
        DWORD NewSize = m_BitSize + m_BitSize;  
        CHAR *NewMem = new char[NewSize];
        assert (NewMem != NULL);

        memset (NewMem, 0, NewSize);

        memcpy (NewMem, m_Bitmap, m_BitSize);
        
        delete m_Bitmap;
        m_Bitmap = NewMem;
        m_BitSize = NewSize;

        return;
    }

public:
    Bitmap (DWORD BitSize)
    {
        m_BitSize = (BitSize>>3) + 1;

        /* algin 8 bytes */
        m_BitSize = ((m_BitSize>>3) + 1)<<3;  
        
        m_Bitmap = new char[m_BitSize];
        assert (m_Bitmap != NULL);

        memset (m_Bitmap, 0, m_BitSize);
         
    }

    ~Bitmap ()
    {
        if (m_Bitmap != NULL)
        {
            delete m_Bitmap;
            m_Bitmap = NULL;
        }
    }

    inline VOID SetBit (DWORD Bit, BOOL Val)
    {
        DWORD No = Bit>>3;
        DWORD Offset = Bit&0x7;

        if (No >= m_BitSize)
        {
            ExtednMemory ();        
        }

        if (Val)
        {
            m_Bitmap[No] |= (1<<Offset);
        }
        else
        {
            m_Bitmap[No] &= ~(1<<Offset);
        }
    }


    inline BOOL CheckBit (DWORD Bit)
    {
        DWORD No = Bit>>3;
        DWORD Offset = Bit&0x7;

        if (No < m_BitSize)
        {
            return (m_Bitmap[No] & (1<<Offset));
        }
        else
        {
            return AF_FALSE;
        }    
    }
};

#endif 

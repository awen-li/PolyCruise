//===- Event.h - Event Type definition ----------------------------------------//
//
//
// Copyright (C) <2019-2024>  <Wen Li>

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
#ifndef _EVENT_H_
#define _EVENT_H_


#define MSG_BEGIN         ('{')
#define MSG_END           ('}')
#define MSG_FP_L          ('(')
#define MSG_FP_R          (')')

#define MSG_VT            (':')
#define MSG_DF            ('=')
#define MSG_MT            (',')

#define VT_INTEGER        ('U')
#define VT_POINTER        ('P')
#define VT_FUNCTION       ('F')
#define VT_GLOBAL         ('G')
#define VT_FPARA          ('A')


/*
    Event Id definition:
    |4b language|4b type|2b soure/sink|18b FunctionId|12b Blockid|24b Instructionid|
*/

#define F_IID2EID(IID)    (IID)
#define F_BID2EID(BID)    (BID << 24)
#define F_FID2EID(FID)    (FID << 36)
#define F_SSD2EID(SSD)    (SSD << 54)
#define F_ETY2EID(ETY)    (ETY << 56)
#define F_LANG2EID(LANG)  (LANG << 60)


#define R_EID2IID(EID)    (unsigned)(EID & 0xFFFFFF)
#define R_EID2BID(EID)    (unsigned)((EID >> 24) & 0xFFF)
#define R_EID2FID(EID)    (unsigned)((EID >> 36) & 0x3FFFF)
#define R_EID2SSD(EID)    (unsigned)((EID >> 54) & 0x3)
#define R_EID2ETY(EID)    (unsigned)((EID >> 56) & 0x0F)
#define R_EID2LANG(EID)   (unsigned)((EID >> 60) & 0x0F)


#define EVENT_DF          (0UL)
#define EVENT_FENTRY      (1UL)
#define EVENT_NR          (2UL)
#define EVENT_BR          (3UL)
#define EVENT_RET         (4UL)
#define EVENT_CALL        (5UL)
#define EVENT_THRC        (6UL)
#define EVENT_GEP         (7UL)
#define EVENT_STORE       (8UL)

#define CLANG_TY          (1UL)
#define PYLANG_TY         (2UL)

#define SOURCE_TY         (1UL)
#define SINK_TY           (2UL)



#endif // _EVENT_H_
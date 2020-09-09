//===-- Event.h - Event Type definition -----------------------------------===//
//
// Copyright (C) <2019-2024>  <Wen Li>
//
//                      The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef _EVENT_H_
#define _EVENT_H_

/*
    Event Id definition:
    |4b language|4b type|20b FunctionId|12b Blockid|24b Instructionid|
*/

#define F_IID2EID(IID)    (IID)
#define F_BID2EID(BID)    (BID << 24)
#define F_FID2EID(FID)    (FID << 36)
#define F_ETY2EID(ETY)    (ETY << 56)
#define F_LANG2EID(LANG)  (LANG << 60)


#define R_EID2IID(EID)    (unsigned)(EID & 0xFFFFFF)
#define R_EID2BID(EID)    (unsigned)((EID >> 24) & 0xFFF)
#define R_EID2FID(EID)    (unsigned)((EID >> 36) & 0xFFFFF)
#define R_EID2ETY(EID)    (unsigned)((EID >> 56) & 0x0F)
#define R_EID2LANG(EID)   (unsigned)((EID >> 56) & 0xF0)


#define EVENT_DF          (0UL)
#define EVENT_FENTRY      (1UL)
#define EVENT_NR          (2UL)
#define EVENT_BR          (3UL)
#define EVENT_RET         (4UL)
#define EVENT_CALL        (5UL)


#define CLANG_TY          (1UL)


#endif // _EVENT_H_

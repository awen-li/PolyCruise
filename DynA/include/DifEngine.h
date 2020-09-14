//===-- DifEngine.h - Dyn information flow engine ---------------------------===//
//
// Copyright (C) <2019-2024>  <Wen Li>
//
//                      The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef _DIFENGINE_H_
#define _DIFENGINE_H_


VOID InitDif ();
VOID DeInitDif ();

DWORD IsEventExist (ULONG Event);
VOID DifEngine (ULONG Event, char *Msg);


#endif // _DIFENGINE_H_
//===- llaf/include/common/BasicMacro.h -   basic macro defined   -------*- C++ -*-===//
//
//                     The LLAF framework
//
// This file is distributed under the University of WSU Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the basic macro for uniformed macros.
//
//===----------------------------------------------------------------------===//
#ifndef _BASICMACRO_H_
#define _BASICMACRO_H_ 
#include "common/BasicType.h"


#define AF_SUCCESS          (0)
#define AF_FAIL             (1)

#define AF_TRUE             (1)
#define AF_FALSE            (0)


#define FUNC_MODULE         ("Func_Module.map")


#define TAINT_NONE          (unsigned)(0)
#define TAINT_RET           (unsigned)(1 << (32-1))
#define TAINT_ARG0          (unsigned)(1 << (32-2))
#define TAINT_ARG1          (unsigned)(1 << (32-3))
#define TAINT_ARG2          (unsigned)(1 << (32-4))
#define TAINT_ARG3          (unsigned)(1 << (32-5))




/////////////////////////////////////////////////////////////////////////////////
#define KNRM  "\x1B[1;0m"
#define KRED  "\x1B[1;31m"
#define KYEL  "\x1B[1;33m"
#define KBLU  "\x1B[1;34m"

#define ErrMsg(msg)   (std::string(KRED) + std::string(msg) + std::string(KNRM))
#define WarnMsg(msg)  (std::string(KYEL) + std::string(msg) + std::string(KNRM))
#define LightMsg(msg) (std::string(KBLU) + std::string(msg) + std::string(KNRM))
/////////////////////////////////////////////////////////////////////////////////


#ifndef DEBUG_MOD
//#define DEBUG_MOD
#endif

#ifdef DEBUG_MOD
#define DEBUG(format, ...) printf(format, ##__VA_ARGS__)
#else
#define DEBUG(format, ...) 
#endif



#endif

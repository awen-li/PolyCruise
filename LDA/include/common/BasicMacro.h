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


#define RET_NO              (unsigned)(1)
#define ARG0_NO             (unsigned)(2)  
#define ARG1_NO             (unsigned)(3) 
#define ARG2_NO             (unsigned)(4) 
#define ARG3_NO             (unsigned)(5) 



#define TAINT_NONE          (unsigned)(0)
#define TAINT_BIT(Bit)      (unsigned)(1 << (32-Bit))

#define TAINT_RET           TAINT_BIT(RET_NO)
#define TAINT_ARG0          TAINT_BIT(ARG0_NO)
#define TAINT_ARG1          TAINT_BIT(ARG1_NO)
#define TAINT_ARG2          TAINT_BIT(ARG2_NO)
#define TAINT_ARG3          TAINT_BIT(ARG3_NO)




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

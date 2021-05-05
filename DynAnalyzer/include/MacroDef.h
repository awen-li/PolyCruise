/***********************************************************
 * Author: Wen Li
 * Date  : 9/01/2020
 * Describe: MacroDef.h - Macro definition 
 * History:
   <1> 9/01/2020 , create
************************************************************/

#ifndef _MACRODEF_H_
#define _MACRODEF_H_ 
#ifdef __cplusplus
extern "C"{
#endif 

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include "TypeDef.h"

#define R_SUCCESS                 (0)
#define R_FAIL                    (1)

#define TRUE                      (1)
#define FALSE                     (0)

#define SHARE_KEY                 (0xC3B3C5D0)

#define ALIGN_8(x)                (((x)%8)?(((x)&~7) + 8):(x))

#define INLINE                    inline


#ifdef __DEBUG__
#define DEBUG(format, ...) printf(format, ##__VA_ARGS__)
#else
#define DEBUG(format, ...) 
#endif


#define mutex_lock_t           pthread_mutex_t
#define mutex_lock_init(x)     pthread_mutex_init(x, NULL)
#define mutex_lock(x)          pthread_mutex_lock(x);
#define mutex_unlock(x)        pthread_mutex_unlock(x);

#define process_lock_t             pthread_rwlock_t
#define process_lock_init(x, attr) pthread_rwlock_init (x, attr) 
#define process_lock(x)            pthread_rwlock_rdlock (x) 
#define process_unlock(x)          pthread_rwlock_unlock (x)



#define PLUGIN_INI             ("/tmp/difg/plugins.ini")

#ifdef __cplusplus
}
#endif

#endif


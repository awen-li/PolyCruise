//===- VisitDir.cpp - visit all files in specified directory -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//

#include "common/VisitDir.h"

using namespace std;

string VisitDir::getSuffix (const CHAR* FileName)
{
    string file = FileName;

    return file.substr(file.find_last_of('.') + 1);
}

VOID VisitDir::PathVisit(const CHAR* strDir, VOID* Para1)
{
    INT Ret;
    DIR *pstDir;                       
    struct dirent *entry;          
    struct stat statbuf;
        
    if((pstDir = opendir(strDir)) == NULL) 
    {  
        printf("%s open fail!! \r\n", strDir);  
        return;  
    }

    //printf("--------[DIR] %s open \r\n", strDir); 
        
    Ret = chdir (strDir);
    assert (Ret == 0);
    while((entry = readdir(pstDir)) != NULL) 
    {
            
        lstat(entry->d_name, &statbuf);
            
        if(S_ISDIR(statbuf.st_mode)) 
        {
    
            if (entry->d_name[0] == '.')
            {
                continue;
            }
      
            PathVisit(entry->d_name, Para1);
        }  
        else 
        {
            runOnFile(entry->d_name, Para1);
        }
    }    
    Ret = chdir(".."); 
    assert (Ret == 0);
    
    closedir(pstDir);
        
    return;
}


VOID VisitDir::Visit(const CHAR* strDir, VOID* Para1)
{
    assert(getcwd(m_CurPath, sizeof(m_CurPath)) != NULL);
    PathVisit(strDir, Para1);
    assert(chdir(m_CurPath) == 0);

}




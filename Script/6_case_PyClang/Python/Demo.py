#!/usr/bin/python
import os
from PyDemo import *

if __name__ == '__main__':
    #Cmd1 = os.getenv ("CMD1")
    #Ret = pyEXEC(Cmd1)
    #print ("Cmd1 Ret = ", Ret)
    
    Cmd2 = os.getenv ("CMD2")
    Ret = pySystem(Cmd2)
    print ("Cmd2 Ret = ", Ret)

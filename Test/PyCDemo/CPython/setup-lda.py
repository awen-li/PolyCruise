#!/usr/bin/python

import os
from distutils.core import setup, Extension

os.environ["CC"]  = "clang -emit-llvm"
os.environ["CXX"] = "clang -emit-llvm"
os.environ["LDSHARED"] = "clang -flto -shared"

module1 = Extension('Trace',
                    define_macros = [('MAJOR_VERSION', '1'), ('MINOR_VERSION', '0')],
                    #extra_link_args=[]
                    #extra_compile_args=[]
                    include_dirs = ['../C/include'],
                    libraries = ['DemoTrace'],
                    library_dirs = ['/usr/lib'],
                    sources = ['Demo.c'])

setup (name = 'DemoTrace',
       version = '1.0',
       description = 'package for python tracing',
       author = 'Wen Li',
       author_email = 'li.wen@wsu.edu',
       ext_modules = [module1])


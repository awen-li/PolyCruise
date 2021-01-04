#!/usr/bin/python

import os
from distutils.core import setup, Extension

os.environ["CC"]  = "clang -Xclang -load -Xclang llvmLDIpass.so"
os.environ["CXX"] = "clang -Xclang -load -Xclang llvmLDIpass.so"
os.environ["LDSHARED"] = "ld -pthread -shared"

module1 = Extension('Trace',
                    define_macros = [('MAJOR_VERSION', '1'), ('MINOR_VERSION', '0')],
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


#!/usr/bin/python

import os
from distutils.core import setup, Extension


module1 = Extension('PyDemo',
                    define_macros = [('MAJOR_VERSION', '1'), ('MINOR_VERSION', '0')],
                    #extra_link_args=[]
                    #extra_compile_args=[]
                    include_dirs = [],
                    #libraries = ['DemoTrace'],
                    #library_dirs = ['/usr/lib'],
                    sources = ['Demo.c'])

setup (name = 'PyDemo',
       version = '1.0',
       description = 'package for capsule setName',
       author = 'Wen xx',
       author_email = 'xx.wen@xxx.edu',
       ext_modules = [module1])


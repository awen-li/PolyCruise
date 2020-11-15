from distutils.core import setup, Extension

demo_module = Extension('demo', sources = ['demo.c'])

setup (name = 'a demo extension module',
       version = '1.0',
       description = 'a demo package',
       ext_modules = [demo_module])

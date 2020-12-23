from distutils.core import setup, Extension

module1 = Extension('PyTrace',
                    define_macros = [('MAJOR_VERSION', '1'),
                                     ('MINOR_VERSION', '0')],
                    include_dirs = ['include'],
                    libraries = ['Trace'],
                    library_dirs = ['.'],
                    sources = ['source/PyTrace.c'])

setup (name = 'PyTrace',
       version = '1.0',
       description = 'package for python tracing',
       author = 'Wen Li',
       author_email = 'li.wen@wsu.edu',
       ext_modules = [module1])

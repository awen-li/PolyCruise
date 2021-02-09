"""
Japronto
"""
import codecs
import os
import re

from setuptools import setup, find_packages

import build

version = '0.1.2'

os.environ["CC"]  = "clang -emit-llvm"
os.environ["CXX"] = "clang"
os.environ["LDSHARED"] = "clang -flto -shared"

    
setup(
    name='japronto',
    version=version,
    url='http://github.com/squeaky-pl/japronto/',
    license='MIT',
    author='Paweł Piotr Przeradowski',
    author_email='przeradowski@gmail.com',
    description='A HTTP application toolkit and server bundle ' +
                'based on uvloop and picohttpparser',
    package_dir={'': 'src'},
    packages=find_packages('src'),
    keywords=['web', 'asyncio'],
    platforms='x86_64 Linux and MacOS X',
    install_requires=[
        'uvloop>=0.11.3',
    ],
    entry_points="""
         [console_scripts]
         japronto = japronto.__main__:main
    """,
    classifiers=[
        'Development Status :: 2 - Pre-Alpha',
        'Intended Audience :: Developers',
        'Environment :: Web Environment',
        'License :: OSI Approved :: MIT License',
        'Operating System :: MacOS :: MacOS X',
        'Operating System :: POSIX :: Linux',
        'Programming Language :: C',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: Implementation :: CPython',
        'Topic :: Internet :: WWW/HTTP'
    ],
    zip_safe=False,
    include_package_data=True,
    package_data={'picohttpparser': ['*.so']},
    ext_modules=build.get_platform(),
    cmdclass={'build_ext': build.custom_build_ext}
)

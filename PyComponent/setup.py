#!/usr/bin/env python
# -*- encoding: utf-8 -*-

from setuptools import setup
from setuptools import find_packages

with open("README.md", "r") as fh:
    long_description = fh.read()

setup(
    name="PyInspect",
    version="0.0.1",
    author="Wen Li",
    author_email="li.wen@wsu.edu",
    description="PyInspect is a dynamic analysis tool accross Python and C.",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/Daybreak2019/PolyCruise.git",
    packages=['PyInspect']
)

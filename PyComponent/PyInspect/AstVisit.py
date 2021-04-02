#!/usr/bin/python
# _*_ coding:utf-8 _*_

import re
import ast
from ast import *
from copy import deepcopy, copy

class ASTWalk(NodeVisitor):

    def __init__(self):

    def visit(self, node):
        """Visit a node."""
        if node is None:
            return
        method = 'visit_' + node.__class__.__name__.lower()
        visitor = getattr(self, method, self.generic_visit)
        return visitor(node)

    def visit_functiondef(self, node, expand=False):
        pass

    def visit_call(self, node, needtarget=True):
        pass

    
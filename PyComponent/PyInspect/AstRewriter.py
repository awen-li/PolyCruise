#!/usr/bin/python
# _*_ coding:utf-8 _*_

__author__ = 'gui'

import re
import ast
from ast import *
from copy import deepcopy, copy

#################################################################
#website: https://sites.google.com/site/pypredictor/
#
#@inproceedings{xu2016python,
#  title={Python predictive analysis for bug detection},
#  author={Xu, Zhaogui and Liu, Peng and Zhang, Xiangyu and Xu, Baowen},
#  booktitle={Proceedings of the 2016 24th ACM SIGSOFT International Symposium on Foundations of Software Engineering},
#  pages={121--132},
#  year={2016}
#}
#################################################################
# Wen Li: adapt to Python3.7
#################################################################

class ASTVisitor(NodeTransformer):

    def __init__(self, lineno=0, col_offset=0, nestedexpand=False):
        self._nestedexpand = nestedexpand # flag for whether expand the sub-scope code nestedly
        self._lineno = lineno   # current line number
        self._nameno = 0        # current temporary name number
        self._oldlineno = -1    # current line number of original code
        self._codelist = []     # current list for adding new generated stmt
        self._lineno2ids = {}   # dictionary of line number to id-string s
        self._lineno2stmt = {}  # dictionary of line number to new stmt
        self._col_offset = col_offset   # current col_offset to set
        self._bodylist = []     # current stmt body (equals to the original body of the compound stmt)
        self._stmtnode = None   # current visit stmt which belongs to the current body
        self._newlineno2oldlineno = {}  # dictionary of mapping new generated stmt to original
        self._underclassdef = False
        self._currclassname = ''

    @property
    def lineno2ids(self):
        return self._lineno2ids

    @property
    def lineno2stmt(self):
        return self._lineno2stmt

    @property
    def newlineno2oldlineno(self):
        return self._newlineno2oldlineno

    def _new_nameno(self):
        self._nameno += 1
        return self._nameno

    def _new_lineno(self):
        self._lineno += 1
        return self._lineno

    def _new_tmp_name(self,  ctx):
        return Name(id='v'+str(self._new_nameno()), ctx=ctx)

    def _new_spec_tmp_name(self, base, ctx):
        return Name(id=('_'+str(base)), ctx=ctx)

    def _add_to_codelist(self, s):
        self._codelist.append(s)
        self._add_to_lineno2stmt(s.lineno, s)
        self._add_to_nl2ol(s.lineno)

    def _add_to_lineno2stmt(self, lineno, stmt):
        self._lineno2stmt[lineno] = stmt

    def _add_to_nl2ol(self, nl):
        self._newlineno2oldlineno[nl] = self._oldlineno

    def _add_to_lineno2ids(self, lineno, ids):
        if self._lineno2ids.get(lineno) is None:
            self._lineno2ids[lineno] = set()
        self._lineno2ids[lineno].update(ids)

    class _GetUsesVisitor(NodeVisitor):
        def __init__(self):
            self.ids = set()

        def visit_Name(self, node):
            self.ids.add(node.id)

    def _get_ids(self, node):
        visitor = self._GetUsesVisitor()
        visitor.visit(node)
        return visitor.ids

    def visit(self, node):
        """Visit a node."""
        if node is None:
            #print ("node is None")
            return
        if hasattr(node, 'lineno'):
            self._oldlineno = node.lineno
        method = 'visit_' + node.__class__.__name__.lower()
        visitor = getattr(self, method, self.generic_visit)
        #if visitor.__name__ == "generic_visit":
        #    print ("@@@@@ visitor ", method, " not defined yet, enter generic_visit...")
        #print (method, end=", ")
        #if isinstance (node, str):
        #    print (node, "\r\n----------------------------------------\r\n")
        #else:
        #    print (ast.dump (node), "\r\n----------------------------------------\r\n")
        return visitor(node)

    def visit_module(self, node):
        if get_docstring(node):
            doc = Expr(value=Str(s=node.body[0].value.s),
                       lineno=self._new_lineno(),
                       col_offset=self._col_offset)
            fix_missing_locations(doc)
            self._add_to_codelist(doc)
            body = node.body[1:]
        else:
            body = list(node.body)
        # add import __builtin__
        if body:
            import_ = Import(names=[alias(name='builtins', asname=None)])
            for i, stmt in enumerate(body):
                if isinstance(stmt, (Import, ImportFrom)) and ('__future__' in dump(stmt)):
                    continue
                else:
                    body.insert(i, import_)
                    break
        else:
            body.append(Pass())
        for s in body:
            self.visit(s)
        return Module(body=self._codelist)

    # Statements
    def visit_assert(self, node):
        test, msg = self.visit(node.test), self.visit(node.msg)
        assert_ = Assert(test=test,
                         msg=msg,
                         lineno=self._new_lineno(),
                         col_offset=self._col_offset)
        fix_missing_locations(assert_)
        self._add_to_codelist(assert_)
        self._add_to_lineno2ids(self._lineno, self._get_ids(assert_))

    def _cache_value(self, val):
        if isinstance(val, (Tuple, List)):
            return val.__class__(elts=[self._cache_value(elt) for elt in val.elts],
                                 ctx=val.ctx,
                                 lineno=val.lineno,
                                 col_offset=val.col_offset)
        else:
            assign = Assign(targets=[self._new_tmp_name(Store())],
                            value=self.visit(val),
                            lineno=self._new_lineno(),
                            col_offset=self._col_offset)
            fix_missing_locations(assign)
            self._add_to_codelist(assign)
            self._add_to_lineno2ids(self._lineno, self._get_ids(assign))
            return Name(id=assign.targets[0].id, ctx=Load())

    def has_star (self, targets):
        target = targets[0]
        if not isinstance(target, (List, Tuple)):
            return False
        for eml in target.elts:
            if isinstance(eml, Starred):
                return True
        return False
    
    def visit_assign(self, node):
        #TODO: we need to handle the defect when assign is like:
        # x, y = (a for a in [1,2])
        # x, y = {1:2,3:4}.itervalues(), the right hand is an iterator
        if self.has_star (node.targets):   
            assign = Assign(targets=node.targets,
                            value=node.value,
                            lineno=self._new_lineno(),
                            col_offset=self._col_offset)
            fix_missing_locations(assign)
            self._add_to_codelist(assign)
            self._add_to_lineno2ids(self._lineno, self._get_ids(assign))
            return
        
        if len(node.targets) > 1:
            val = self._cache_value(node.value)
            if not isinstance(val, Name):
                val = self.visit(val)
            for tgt in node.targets:
                self.visit(Assign(targets=[tgt],
                                  value=val))
        else:
            tgt, val = node.targets[0], node.value
            if isinstance(tgt, (List, Tuple)):
                val = self._cache_value(val)
                if isinstance(val, (Tuple, List)):
                    t2v = zip(tgt.elts, val.elts)
                    for t, v in t2v:
                        self.visit(Assign(targets=[t],
                                          value=v))
                else:
                    for i, t in enumerate(tgt.elts):
                        self.visit(Assign(targets=[t],
                                          value=Subscript(value=val,
                                                          slice=Index(value=Num(n=i)),
                                                          ctx=Load())))
            else:
                assign = Assign(targets=[self.visit(tgt)],
                                value=self.visit(val),
                                lineno=self._new_lineno(),
                                col_offset=self._col_offset)
                fix_missing_locations(assign)
                self._add_to_codelist(assign)
                self._add_to_lineno2ids(self._lineno, self._get_ids(assign))

    def visit_augassign(self, node):
        self.visit_augassign1(node)

    def visit_augassign1(self, node):
        # for x.y += 1, we transform it to $=x.y, $+=\n1, x.y=$
        tgt, val = self.visit(node.target), self.visit(node.value)
        if not isinstance(tgt, Name):
            tmp_tgt, tmp_val = self._new_tmp_name(Store()), deepcopy(tgt)
            tmp_val.ctx = Load()
            # $ = x.y
            tmp_assign = Assign(targets=[tmp_tgt],
                                value=tmp_val,
                                lineno=self._new_lineno(),
                                col_offset=self._col_offset)
            fix_missing_locations(tmp_assign)
            self._add_to_codelist(tmp_assign)
            self._add_to_lineno2ids(self._lineno, self._get_ids(tmp_assign))
            # $$ = $
            flag_assign = Assign(targets=[self._new_spec_tmp_name(tmp_tgt.id, Store())],
                                 value=Name(id=tmp_tgt.id, ctx=Load()),
                                 lineno=self._new_lineno(),
                                 col_offset=self._col_offset)
            fix_missing_locations(flag_assign)
            self._add_to_codelist(flag_assign)
            self._add_to_lineno2ids(self._lineno, self._get_ids(flag_assign))
            # $ += $0
            aug_assign = AugAssign(target=tmp_tgt,  # AugLoad is not used in python2.x
                                   op=node.op,
                                   value=val,
                                   lineno=self._new_lineno(),   # the aug_assign stmt
                                   col_offset=self._col_offset)
            fix_missing_locations(aug_assign)
            self._add_to_codelist(aug_assign)
            self._add_to_lineno2ids(self._lineno, self._get_ids(aug_assign))
        else:
            # $$ = n
            flag_assign = Assign(targets=[self._new_spec_tmp_name(tgt.id, Store())],
                                 value=Name(id=tgt.id, ctx=Load()),
                                 lineno=self._new_lineno(),
                                 col_offset=self._col_offset)
            fix_missing_locations(flag_assign)
            self._add_to_codelist(flag_assign)
            self._add_to_lineno2ids(self._lineno, self._get_ids(flag_assign))
            # $ += n
            aug_assign = AugAssign(target=tgt,  # AugLoad is not used in python2.x
                                   op=node.op,
                                   value=val,
                                   lineno=self._new_lineno(),   # the aug_assign stmt
                                   col_offset=self._col_offset)
            fix_missing_locations(aug_assign)
            self._add_to_codelist(aug_assign)
            self._add_to_lineno2ids(self._lineno, self._get_ids(aug_assign))
        if not isinstance(tgt, Name):
            # x.y = $
            tmp_tgt, tmp_val = deepcopy(tgt), copy(tmp_tgt)
            tmp_val.ctx = Load()
            tmp_assign = Assign(targets=[tmp_tgt],
                                value=tmp_val,
                                lineno=self._new_lineno(),
                                col_offset=self._col_offset)
            fix_missing_locations(tmp_assign)
            self._add_to_codelist(tmp_assign)
            self._add_to_lineno2ids(self._lineno, self._get_ids(tmp_assign))

    def visit_augassign2(self, node):
        # for x += 1, we transform it to $=x, $+=\n1, x=$
        # for x.y += 1, we transform it to $=x.y, $+=\n1, x.y=$
        tgt, val = self.visit(node.target), self.visit(node.value)
        tmp_tgt, tmp_val = self._new_tmp_name(Store()), deepcopy(tgt)
        tmp_val.ctx = Load()
        tmp_assign = Assign(targets=[copy(tmp_tgt)],
                            value=tmp_val,
                            lineno=self._new_lineno(),
                            col_offset=self._col_offset)
        fix_missing_locations(tmp_assign)
        self._add_to_codelist(tmp_assign)
        self._add_to_lineno2ids(self._lineno, self._get_ids(tmp_assign))

        aug_assign = Assign(targets=[self._new_tmp_name(Store())],
                            value=BinOp(left=Name(id=tmp_tgt.id, ctx=Load()),
                                        op=node.op,
                                        right=val),
                            lineno=self._new_lineno(),   # the aug_assign stmt
                            col_offset=self._col_offset)
        fix_missing_locations(aug_assign)
        self._add_to_codelist(aug_assign)
        self._add_to_lineno2ids(self._lineno, self._get_ids(aug_assign))

        tmp_tgt, tmp_val = deepcopy(tgt), Name(id=aug_assign.targets[0].id, ctx=Load())
        tmp_val.ctx = Load()
        tmp_assign = Assign(targets=[tmp_tgt],
                            value=tmp_val,
                            lineno=self._new_lineno(),
                            col_offset=self._col_offset)
        fix_missing_locations(tmp_assign)
        self._add_to_codelist(tmp_assign)
        self._add_to_lineno2ids(self._lineno, self._get_ids(tmp_assign))

    def visit_annassign(self, node):
        assign = AnnAssign(target=[self.visit(node.target)],
                           annotation=self.visit(node.annotation),
                           value=self.visit(node.value),
                           simple=node.simple,
                           lineno=self._new_lineno(), 
                           col_offset=self._col_offset)
        fix_missing_locations(assign)
        self._add_to_codelist(assign)
        self._add_to_lineno2ids(self._lineno, self._get_ids(assign))

    def visit_importfrom(self, node):
        importfrom = ImportFrom(module=node.module,
                                names=[alias(name=a.name,
                                             asname=self.visit_identifier(a.asname)) for a in node.names],
                                level=node.level,
                                lineno=self._new_lineno(),
                                col_offset=self._col_offset)
        fix_missing_locations(importfrom)
        self._add_to_codelist(importfrom)

    def visit_import(self, node):
        for alias_ in node.names:
            name, asname = alias_.name, alias_.asname
            import_ = Import(names=[alias(name=name,
                                          asname=self.visit_identifier(asname))],
                             lineno=self._new_lineno(),
                             col_offset=self._col_offset)
            fix_missing_locations(import_)
            self._add_to_codelist(import_)

    def visit_exec(self, node):
        exec_ = Exec(body=self.visit(node.body),
                     globals=self.visit(node.globals),
                     locals=self.visit(node.locals),
                     lineno=self._new_lineno(),
                     col_offset=self._col_offset)
        fix_missing_locations(exec_)
        self._add_to_codelist(exec_)
        self._add_to_lineno2ids(self._lineno, self._get_ids(exec_))

    def visit_expr(self, node):
        node = node.value
        # if isinstance(node, Call):
        #     expr_ = Expr(value=Call(func=self.visit(node.func),
        #                             args=[self.visit(arg) for arg in node.args],
        #                             keywords=[keyword(arg=kd.arg,
        #                                               value=self.visit(kd.value)) for kd in node.keywords],
        #                             starargs=self.visit(node.starargs),
        #                             kwargs=self.visit(node.kwargs)),
        #                  lineno=self._new_lineno(),
        #                  col_offset=self._col_offset)
        #     fix_missing_locations(expr_)
        #     self._add_to_codelist(expr_)
        #     self._add_to_lineno2ids(self._lineno, self._get_ids(expr_))
        if isinstance(node, Yield):
            expr_ = Expr(value=self.visit_yield(node),
                         lineno=self._new_lineno(),
                         col_offset=self._col_offset)
        elif isinstance(node, Call):
            expr_ = Expr(value=self.visit_call(node, False),
                         lineno=self._new_lineno(),
                         col_offset=self._col_offset)
        else:
            expr_ = Expr(value=self.visit(node),
                         lineno=self._new_lineno(),
                         col_offset=self._col_offset)
        fix_missing_locations(expr_)
        self._add_to_codelist(expr_)
        self._add_to_lineno2ids(self._lineno, self._get_ids(expr_))


    def visit_asyncfunctiondef (self, node, expand=False):
        funcdef = AsyncFunctionDef(name=self.visit_identifier(node.name),
                                   args=self.visit(node.args),
                                   body=None,
                                   decorator_list=[self.visit(deco) for deco in node.decorator_list],
                                   lineno=self._new_lineno(),
                                   col_offset=self._col_offset)
        fix_missing_locations(funcdef)
        self._add_to_codelist(funcdef)
        self._add_to_lineno2ids(self._lineno, (node.name,))
        if not (expand or self._nestedexpand):
            funcdef.body = node.body
            self._lineno += (node.body[-1].lineno - node.lineno + 5)
        else:
            ori_code = self._codelist
            self._codelist = []
            self._col_offset += 4
            # case having doc-string
            if get_docstring(node):
                doc = Expr(value=Str(s=node.body[0].value.s),
                           lineno=self._new_lineno(),
                           col_offset=self._col_offset)
                fix_missing_locations(doc)
                self._add_to_codelist(doc)
                body = node.body[1:]
            else:
                body = node.body
            for s in body:
                self.visit(s)
            funcdef.body = self._codelist
            self._codelist = ori_code
            self._col_offset -= 4

    def visit_functiondef(self, node, expand=False):
        #v358 = 'numpy'
        #v359 = array_function_dispatch(_pad_dispatcher, module=v358)
        #@v359
        #def pad(array, pad_width, mode=v357, **kwargs)
        #print (ast.dump (node))
        def _deco_list (decorator_list):
            list = [self.visit(deco) for deco in decorator_list]
            self._lineno += len (list)
            return list
        funcdef = FunctionDef(name=self.visit_identifier(node.name),
                              args=self.visit(node.args),
                              body=None,
                              decorator_list=_deco_list(node.decorator_list),
                              lineno=self._new_lineno(),
                              col_offset=self._col_offset)
        fix_missing_locations(funcdef)
        self._add_to_codelist(funcdef)
        self._add_to_lineno2ids(self._lineno, (node.name,))
        if not (expand or self._nestedexpand):
            funcdef.body = node.body
            self._lineno += (node.body[-1].lineno - node.lineno + 5)
        else:
            ori_code = self._codelist
            self._codelist = []
            self._col_offset += 4
            # case having doc-string
            if get_docstring(node):
                doc = Expr(value=Str(s=node.body[0].value.s),
                           lineno=self._new_lineno(),
                           col_offset=self._col_offset)
                fix_missing_locations(doc)
                self._add_to_codelist(doc)
                body = node.body[1:]
            else:
                body = node.body
            for s in body:
                self.visit(s)
            funcdef.body = self._codelist
            self._codelist = ori_code
            self._col_offset -= 4

    def visit_classdef(self, node, expanded=False):
        def _deco_list (decorator_list):
            list = [self.visit(deco) for deco in decorator_list]
            self._lineno += len (list)
            return list
        classdef = ClassDef(name=self.visit_identifier(node.name),
                            bases=[self.visit(base) for base in node.bases],
                            keywords=[self.visit(kw) for kw in node.keywords],
                            body=node.body,
                            decorator_list=_deco_list(node.decorator_list),
                            lineno=self._new_lineno(),
                            col_offset=self._col_offset)
        fix_missing_locations(classdef)
        #Wen, line number is not consistent with that of runtime.
        #self._lineno += (node.body[-1].lineno-node.lineno+5)
        self._add_to_codelist(classdef)
        self._add_to_lineno2ids(self._lineno, (node.name,))
        if not (expanded or self._nestedexpand):
            classdef.body = node.body
        else:
            oriunderclassdef = self._underclassdef
            oriclassname = self._currclassname
            self._underclassdef = True
            self._currclassname = re.sub('(^_*)', '', node.name)
            ori_code = self._codelist
            self._codelist = []
            self._col_offset += 4
            # case having doc-string
            if get_docstring(node):
                doc = Expr(value=Str(s=node.body[0].value.s),
                           lineno=self._new_lineno(),
                           col_offset=self._col_offset)
                fix_missing_locations(doc)
                self._add_to_codelist(doc)
                body = node.body[1:]
            else:
                body = node.body
            for s in body:
                self.visit(s)
            classdef.body = self._codelist
            self._codelist = ori_code
            self._col_offset -= 4
            self._underclassdef = oriunderclassdef
            self._currclassname = oriclassname

    def visit_if(self, node):
        #print ("[", self._lineno, "]visit_if ----> ", ast.dump (node))
        # add if stmt to the code list
        if_ = If(test=self.visit(node.test),
                 body=None,  # add latter
                 orelse=None,  # add latter
                 lineno=self._new_lineno(),
                 col_offset=self._col_offset)
        fix_missing_locations(if_)
        self._add_to_codelist(if_)
        if isinstance (if_.test, Name):
            self._add_to_lineno2ids(self._lineno, (if_.test.id, ))
        elif isinstance (if_.test, NameConstant):
            self._add_to_lineno2ids(self._lineno, (if_.test.value, ))
        else:
            raise NotImplementedError('visit_if -> unknown type in test.')
        
        ori_code = self._codelist
        # process the body part
        self._codelist = []
        self._col_offset += 4
        #print ("\t If-body, lineno = ", self._lineno)
        for s in node.body:
            self.visit(s)
        if_.body = self._codelist      
        # process the orelse part
        if len(node.orelse) != 0:
            self._lineno += 1
        self._codelist = []
        #print ("\t If-else, lineno = ", self._lineno)
        for s in node.orelse:
            self.visit(s)
        if_.orelse = self._codelist
        # recover it to the outer code list
        self._codelist = ori_code
        self._col_offset -= 4

    def visit_for(self, node):
        # add for stmt into the codelist
        # we call the enumerate function to generate
        # the index for the trace generation
        index_name, tgt_name = self._new_tmp_name(Store()), self._new_tmp_name(Store())
        for_ = For(target=Tuple(elts=[index_name, tgt_name], ctx=Store()),
                   iter=Call(func=Attribute(value=Name(id='builtins', ctx=Load()),
                                            attr='enumerate',
                                            ctx=Load()),
                             args=[self.visit(node.iter)],
                             keywords=[],
                             starargs=None,
                             kwargs=None),
                   body=None,    # add latter
                   orelse=None,  # add latter
                   lineno=self._new_lineno (),
                   col_offset=self._col_offset
                   )
        fix_missing_locations(for_)
        self._add_to_codelist(for_)
        self._add_to_lineno2ids(self._lineno, (index_name.id, tgt_name.id, for_.iter.args[0].id))
        # tgt_name = self._new_tmp_name(Store())
        # for_ = For(target=tgt_name,
        #            iter=self.visit(node.iter),
        #            body=None,    # add latter
        #            orelse=None,  # add latter
        #            lineno=self._new_lineno(),
        #            col_offset=self._col_offset
        #            )
        # fix_missing_locations(for_)
        # self._add_to_codelist(for_)
        # self._add_to_lineno2ids(self._lineno, (tgt_name.id, for_.iter.id))
        ori_code = self._codelist
        # process the body part
        self._codelist = []
        self._col_offset += 4
        tmp_assign = Assign(targets=[deepcopy(node.target)],
                            value=Name(id=tgt_name.id,
                                       ctx=Load()),
                            lineno=self._lineno,
                            col_offset=self._col_offset)
        fix_missing_locations(tmp_assign)
        self.visit(tmp_assign)
        for s in node.body:
            self.visit(s)
        for_.body = self._codelist
        # process the orelse part
        if len(node.orelse) != 0:
            self._lineno += 1
        self._codelist = []
        for s in node.orelse:
            #print ("\tForelse ----> ", ast.dump (s))
            self.visit(s)
        for_.orelse = self._codelist
        #recover to the outer code list
        self._codelist = ori_code
        self._col_offset -= 4

    def visit_while(self, node):
        while_ = While(test=self.visit(node.test),
                       body=None,  # add latter
                       orelse=None,  # add latter
                       lineno=self._new_lineno(),
                       col_offset=self._col_offset)
        fix_missing_locations(while_)
        self._add_to_codelist(while_)
        
        _test = while_.test
        if isinstance (_test, Name):
            self._add_to_lineno2ids(self._lineno, (_test.id,))
        elif isinstance (_test, NameConstant):
            #print ("while.test => NameConstant", ast.dump (_test))
            self._add_to_lineno2ids(self._lineno, (_test.value,))
        else:
            raise NotImplementedError('visit_while -> unknown type in test.')
        
        ori_code = self._codelist
        # process the body part
        self._codelist = []
        self._col_offset += 4
        for s in node.body:
            self.visit(s)
        if not isinstance(node.test, Name) and not isinstance(node.test, NameConstant):
            #print (ast.dump (node.test))
            assign = Assign(targets=[Name(id=while_.test.id, ctx=Store())],
                            value=self.visit(node.test),
                            lineno=self._new_lineno(),
                            col_offset=self._col_offset)
            fix_missing_locations(assign)
            self._add_to_codelist(assign)
            self._add_to_lineno2ids(self._lineno, self._get_ids(assign))
        while_.body = self._codelist
        # process the orelse part
        #self._lineno += 1
        self._codelist = []
        for s in node.orelse:
            self.visit(s)
        while_.orelse = self._codelist
        # recover it to the outer code list
        self._codelist = ori_code
        self._col_offset -= 4

    def visit_with(self, node):
        context_name = self.visit(node.items[0].context_expr)
        if node.items[0].optional_vars is None or isinstance(node.items[0].optional_vars, Name):
            optional_vars_name = self.visit(node.items[0].optional_vars)
            flag = False
        else:
            optional_vars_name = self._new_tmp_name(Store())
            flag = True

        with_ = With(context_expr=context_name,
                     optional_vars=optional_vars_name,
                     body=None,  # add latter
                     lineno=self._new_lineno(),
                     col_offset=self._col_offset)
        fix_missing_locations(with_)
        self._add_to_codelist(with_)
        self._add_to_lineno2ids(self._lineno, (with_.context_expr.id,))
        if with_.optional_vars is not None:
            self._add_to_lineno2ids(self._lineno, (with_.optional_vars.id,))
        ori_code = self._codelist
        # process the body part
        self._codelist = []
        self._col_offset += 4
        if flag:
            tmp_assign = Assign(targets=[deepcopy(node.items[0].optional_vars)],
                                value=Name(id=optional_vars_name.id,
                                           ctx=Load()),
                                lineno=self._new_lineno(),
                                col_offset=self._col_offset)
            fix_missing_locations(tmp_assign)
            self.visit(tmp_assign)
        for s in node.body:
            self.visit(s)
        with_.body = self._codelist
        # recover it to the outer code list
        self._codelist = ori_code
        self._col_offset -= 4

    def visit_pass(self, node):
        pass_ = Pass(lineno=self._new_lineno(),
                     col_offset=self._col_offset)
        self._add_to_codelist(pass_)

    def visit_print(self, node):
        print_ = Print(dest=self.visit(node.dest),
                       values=[self.visit(val) for val in node.values],
                       nl=node.nl,
                       lineno=self._new_lineno(),
                       col_offset=self._col_offset)
        fix_missing_locations(print_)
        self._add_to_codelist(print_)
        self._add_to_lineno2ids(self._lineno, self._get_ids(print_))

    def visit_delete(self, node):
        delete = Delete(targets=[self.visit(tgt) for tgt in node.targets],
                        lineno=self._new_lineno(),
                        col_offset=self._col_offset)
        fix_missing_locations(delete)
        self._add_to_codelist(delete)

    def visit_try(self, node):
        #print (ast.dump (node))
        type_names_body = []
        for handler in node.handlers:
            type_names_body.append((self.visit(handler.type),
                                    self.visit(handler.name),
                                    handler.body))

        tryexp = Try(body=None,    # add latter
                     handlers=[],  # add latter
                     orelse=None,  # add latter
                     finalbody=None, # add latter
                     lineno=self._new_lineno(),
                     col_offset=self._col_offset)
        fix_missing_locations(tryexp)
        self._add_to_codelist(tryexp)
        ori_code = self._codelist
        # process the body part
        self._codelist = []
        self._col_offset += 4
        for s in node.body:
            self.visit(s)
        tryexp.body = self._codelist
        # process except handlers
        self._col_offset -= 4
        for t, n, b in type_names_body:
            eh = ExceptHandler(type=t,
                               name=n,
                               body=None,
                               lineno=self._new_lineno(),
                               col_offset=self._col_offset)
            fix_missing_locations(eh)
            self._add_to_lineno2stmt(eh.lineno, eh)
            tryexp.handlers.append(eh)
            self._add_to_lineno2ids(self._lineno, [x.id for x in [t, n]
                                                   if isinstance(x, Name)])
            # process the handler body stmts
            self._codelist = []
            self._col_offset += 4
            for s in b:
                self.visit(s)
            eh.body = self._codelist
            self._col_offset -= 4
        # process the orelse part
        if len (node.orelse) != 0:
            self._lineno += 1
        self._codelist = []
        self._col_offset += 4
        for s in node.orelse:
            self.visit(s)
        tryexp.orelse = self._codelist
        # process the finally part
        if len (node.finalbody) != 0:
            self._lineno += 1
        self._codelist = []
        self._col_offset += 4
        for s in node.finalbody:
            self.visit(s)
        tryexp.finalbody = self._codelist
        # recover it to the outer code list
        self._codelist = ori_code
        self._col_offset -= 4

    
    def visit_tryexcept(self, node):
        type_names_body = []
        for handler in node.handlers:
            type_names_body.append((self.visit(handler.type),
                                    self.visit(handler.name),
                                    handler.body))

        tryexp = TryExcept(body=None,    # add latter
                           handlers=[],  # add latter
                           orelse=None,  # add latter
                           lineno=self._new_lineno(),
                           col_offset=self._col_offset)
        fix_missing_locations(tryexp)
        self._add_to_codelist(tryexp)
        ori_code = self._codelist
        # process the body part
        self._codelist = []
        self._col_offset += 4
        for s in node.body:
            self.visit(s)
        tryexp.body = self._codelist
        # process except handlers
        self._col_offset -= 4
        for t, n, b in type_names_body:
            eh = ExceptHandler(type=t,
                               name=n,
                               body=None,
                               lineno=self._new_lineno(),
                               col_offset=self._col_offset)
            fix_missing_locations(eh)
            self._add_to_lineno2stmt(eh.lineno, eh)
            tryexp.handlers.append(eh)
            self._add_to_lineno2ids(self._lineno, [x.id for x in [t, n]
                                                   if isinstance(x, Name)])
            # process the handler body stmts
            self._codelist = []
            self._col_offset += 4
            for s in b:
                self.visit(s)
            eh.body = self._codelist
            self._col_offset -= 4
        # process the orelse part
        #self._lineno += 1
        self._codelist = []
        self._col_offset += 4
        for s in node.orelse:
            self.visit(s)
        tryexp.orelse = self._codelist
        # recover it to the outer code list
        self._codelist = ori_code
        self._col_offset -= 4

    def visit_tryfinally(self, node):
        tryfinally = TryFinally(body=None,
                                finalbody=None,
                                lineno=self._new_lineno(),
                                col_offset=self._col_offset)
        fix_missing_locations(tryfinally)
        self._add_to_codelist(tryfinally)
        ori_code = self._codelist
        # process the body part
        self._codelist = []
        self._col_offset += 4
        for s in node.body:
            self.visit(s)
        tryfinally.body = self._codelist
        # process the orelse part
        #self._lineno += 1
        self._codelist = []
        for s in node.finalbody:
            self.visit(s)
        tryfinally.finalbody = self._codelist
        # recover it to the outer code list
        self._codelist = ori_code
        self._col_offset -= 4

    def visit_global(self, node):
        global_ = Global(names=[self.visit_identifier(name) for name in node.names],
                         lineno=self._new_lineno(),
                         col_offset=self._col_offset)
        self._add_to_codelist(global_)
        self._add_to_lineno2ids(self._lineno, node.names)

    def visit_nonlocal(self, node):
        nonl_ = Nonlocal(names=node.names,
                         lineno=self._new_lineno(),
                         col_offset=self._col_offset)
        fix_missing_locations(nonl_)
        self._add_to_codelist(nonl_)
        #raise NotImplementedError('Nonlocal')

    def visit_return(self, node):
        return_ = Return(value=self.visit(node.value),
                         lineno=self._new_lineno(),
                         col_offset=self._col_offset)
        fix_missing_locations(return_)
        self._add_to_codelist(return_)
        if isinstance(return_.value, Name):
            self._add_to_lineno2ids(self._lineno, (return_.value.id,))

    def visit_break(self, node):
        break_ = Break(lineno=self._new_lineno(),
                       col_offset=self._col_offset)
        self._add_to_codelist(break_)

    def visit_continue(self, node):
        continue_ = Continue(lineno=self._new_lineno(),
                             col_offset=self._col_offset)
        self._add_to_codelist(continue_)

    # Raise(expr? exc, expr? cause)
    def visit_raise(self, node):
        raise_ = Raise(exc=self.visit(node.exc),
                       cause=self.visit(node.cause),
                       #tback=self.visit(node.tback),
                       lineno=self._new_lineno(),
                       col_offset=self._col_offset)
        fix_missing_locations(raise_)
        self._add_to_codelist(raise_)
        #self._add_to_lineno2ids(self._new_lineno(), [x.id for x in
        #                                             (node.type, node.inst, node.tback)
        #                                             if isinstance(x, Name)])

    # Expressions

    def visit_attribute(self, node):
        if isinstance(node.ctx, (Load, Param)):
            assign = Assign(targets=[self._new_tmp_name(Store())],
                            value=Attribute(value=self.visit(node.value),
                                            attr=self.visit_identifier(node.attr),
                                            ctx=Load()),
                            lineno=self._new_lineno(),
                            col_offset=self._col_offset)
            fix_missing_locations(assign)
            self._add_to_codelist(assign)
            self._add_to_lineno2ids(self._lineno, self._get_ids(assign))
            return Name(id=assign.targets[0].id,
                        ctx=node.ctx)
        elif isinstance(node.ctx, (Store, Del)):
            return Attribute(value=self.visit(node.value),
                             attr=self.visit_identifier(node.attr),
                             ctx=node.ctx)
        else:
            raise NotImplementedError('unknown context.')

    def visit_call(self, node, needtarget=True):
        call_args = [self.visit(arg) for arg in node.args]
        keywords_list = [keyword(arg=self.visit_identifier(kd.arg),
                         value=self.visit(kd.value)) for kd in node.keywords]
        if isinstance(node.func, Attribute):
            func_name = Attribute(value=self.visit(node.func.value),
                                  attr=self.visit_identifier(node.func.attr),
                                  ctx=Load())
        else:
            func_name = self.visit(node.func) 
            if func_name.id == "next":
                iter_assign = Assign(targets=[self._new_tmp_name(Store())], 
                                      value=Call(func=Name(id='iter', ctx=Load()),
                                                  args=call_args,
                                                  keywords=keywords_list),
                                      lineno=self._new_lineno(),
                                      col_offset=self._col_offset
                                    )
                fix_missing_locations(iter_assign)
                self._add_to_codelist(iter_assign)
                self._add_to_lineno2ids(self._lineno, self._get_ids(iter_assign))
                call_args = iter_assign.targets

        call = Call(func=func_name,
                    args=call_args,
                    keywords=keywords_list)
        if needtarget:
            assign = Assign(targets=[self._new_tmp_name(Store())],
                            value=call,
                            lineno=self._new_lineno(),
                            col_offset=self._col_offset)
            fix_missing_locations(assign)
            self._add_to_codelist(assign)
            self._add_to_lineno2ids(self._lineno, self._get_ids(assign))
            if not isinstance(node.func, Attribute) and func_name.id == "zip":
                zip_assign = Assign(targets=[self._new_tmp_name(Store())], 
                                     value=Call(func=Name(id='list', ctx=Load()),
                                                 args=[Name(id=assign.targets[0].id, ctx=Load())],
                                                 keywords=[]),
                                     lineno=self._new_lineno(),
                                     col_offset=self._col_offset
                                    )
                fix_missing_locations(zip_assign)
                self._add_to_codelist(zip_assign)
                self._add_to_lineno2ids(self._lineno, self._get_ids(zip_assign))
                assign = zip_assign
            return Name(id=assign.targets[0].id, ctx=Load())
        else:
            return call

    def visit_name(self, node):
        if node.id in ('True', 'False', 'None') and (not isinstance(node.ctx, Store)):
            assign = Assign(targets=[self._new_tmp_name(Store())],
                            value=Name(id=node.id, ctx=Load()),
                            lineno=self._new_lineno(),
                            col_offset=self._col_offset)
            fix_missing_locations(assign)
            self._add_to_codelist(assign)
            self._add_to_lineno2ids(self._lineno, self._get_ids(assign))
            return Name(id=assign.targets[0].id, ctx=Load())
        return Name(id=self.visit_identifier(node.id), ctx=node.ctx)

    def visit_identifier(self, id_):
        if (id_ is not None) and self._underclassdef:
            if id_.startswith('__') and (not id_.endswith('__')):
                id_ = '_' + self._currclassname + id_
        return id_

    def visit_str(self, node):
        if isinstance (node, str):
            return node
        assign = Assign(targets=[self._new_tmp_name(Store())],
                        value=Str(s=node.s),
                        lineno=self._new_lineno(),
                        col_offset=self._col_offset)
        fix_missing_locations(assign)
        self._add_to_codelist(assign)
        self._add_to_lineno2ids(self._lineno, self._get_ids(assign))
        return Name(id=assign.targets[0].id, ctx=Load())

    def visit_bytes(self, node):
        bytes = Bytes(s=node.s,
                      lineno=self._new_lineno(),
                      col_offset=self._col_offset)
        fix_missing_locations(bytes)
        #self._add_to_codelist(bytes)
        return bytes
        #raise NotImplementedError('bytes')

    def visit_num(self, node):
        assign = Assign(targets=[self._new_tmp_name(Store())],
                        value=Num(n=node.n),
                        lineno=self._new_lineno(),
                        col_offset=self._col_offset)
        fix_missing_locations(assign)
        self._add_to_codelist(assign)
        self._add_to_lineno2ids(self._lineno, self._get_ids(assign))
        return Name(id=assign.targets[0].id, ctx=Load())

    def visit_tuple(self, node):
        if isinstance(node.ctx, (Load, Param)):
            assign = Assign(targets=[self._new_tmp_name(Store())],
                            value=Tuple(elts=[self.visit(elt) for elt in node.elts],
                                        ctx=node.ctx),
                            lineno=self._new_lineno(),
                            col_offset=self._col_offset)
            fix_missing_locations(assign)
            self._add_to_codelist(assign)
            self._add_to_lineno2ids(self._lineno, self._get_ids(assign))
            # assign = Assign(targets=[self._new_tmp_name(Store())],
            #                 value=Call(func=Attribute(value=Name(id='__builtin__',
            #                                                      ctx=Load()),
            #                                           attr='tuple',
            #                                           ctx=Load()),
            #                            args=[Name(id=assign.targets[0].id,
            #                                       ctx=Load())],
            #                            keywords=[],
            #                            starargs=None,
            #                            kwargs=None),
            #                 lineno=self._new_lineno(),
            #                 col_offset=self._col_offset)
            # fix_missing_locations(assign)
            # self._add_to_codelist(assign)
            # self._add_to_lineno2ids(self._lineno, self._get_ids(assign))
            return Name(id=assign.targets[0].id, ctx=node.ctx)
        elif isinstance(node.ctx, (Store, Del)):
            return Tuple(elts=[self.visit(elt) for elt in node.elts], ctx=node.ctx)
        else:
            raise NotImplementedError('unknown context')

    def visit_list(self, node):
        if isinstance(node.ctx, (Load, Param)):
            assign = Assign(targets=[self._new_tmp_name(Store())],
                            value=List(elts=[self.visit(elt) for elt in node.elts],
                                       ctx=node.ctx),
                            lineno=self._new_lineno(),
                            col_offset=self._col_offset)
            fix_missing_locations(assign)
            self._add_to_codelist(assign)
            self._add_to_lineno2ids(self._lineno, self._get_ids(assign))
            # assign = Assign(targets=[self._new_tmp_name(Store())],
            #                 value=Call(func=Attribute(value=Name(id='__builtin__',
            #                                                      ctx=Load()),
            #                                           attr='list',
            #                                           ctx=Load()),
            #                            args=[Name(id=assign.targets[0].id,
            #                                       ctx=Load())],
            #                            keywords=[],
            #                            starargs=None,
            #                            kwargs=None),
            #                 lineno=self._new_lineno(),
            #                 col_offset=self._col_offset)
            # fix_missing_locations(assign)
            # self._add_to_codelist(assign)
            # self._add_to_lineno2ids(self._lineno, self._get_ids(assign))
            return Name(id=assign.targets[0].id, ctx=node.ctx)
        elif isinstance(node.ctx, (Store, Del)):
            return List(elts=[self.visit(elt) for elt in node.elts], ctx=node.ctx)
        else:
            raise NotImplementedError('unknown context')

    def visit_set(self, node):
        assign = Assign(targets=[self._new_tmp_name(Store())],
                        value=Set(elts=[self.visit(elt) for elt in node.elts]),
                        lineno=self._new_lineno(),
                        col_offset=self._col_offset)
        fix_missing_locations(assign)
        self._add_to_codelist(assign)
        self._add_to_lineno2ids(self._lineno, self._get_ids(assign))
        assign = Assign(targets=[self._new_tmp_name(Store())],
                        value=Call(func=Name(id='set', ctx=Load()),
                                    args=[Name(id=assign.targets[0].id, ctx=Load())],
                                    keywords=[]),
                        lineno=self._new_lineno(),
                        col_offset=self._col_offset)
        fix_missing_locations(assign)
        self._add_to_codelist(assign)
        self._add_to_lineno2ids(self._lineno, self._get_ids(assign))
        return Name(id=assign.targets[0].id, ctx=Load())

    def visit_dict(self, node):
        assign = Assign(targets=[self._new_tmp_name(Store())],
                        value=Dict(keys=[self.visit(key) for key in node.keys],
                                   values=[self.visit(val) for val in node.values]),
                        lineno=self._new_lineno(),
                        col_offset=self._col_offset)
        fix_missing_locations(assign)
        self._add_to_codelist(assign)
        self._add_to_lineno2ids(self._lineno, self._get_ids(assign))
        # assign = Assign(targets=[self._new_tmp_name(Store())],
        #                 value=Call(func=Attribute(value=Name(id='__builtin__',
        #                                                      ctx=Load()),
        #                                           attr='dict',
        #                                           ctx=Load()),
        #                            args=[Name(id=assign.targets[0].id,
        #                                       ctx=Load())],
        #                            keywords=[],
        #                            starargs=None,
        #                            kwargs=None),
        #                 lineno=self._new_lineno(),
        #                 col_offset=self._col_offset)
        # fix_missing_locations(assign)
        # self._add_to_codelist(assign)
        # self._add_to_lineno2ids(self._lineno, self._get_ids(assign))
        return Name(id=assign.targets[0].id, ctx=Load())

    def visit_binop(self, node):
        assign = Assign(targets=[self._new_tmp_name(Store())],
                        value=BinOp(left=self.visit(node.left),
                                    op=node.op,
                                    right=self.visit(node.right)),
                        lineno=self._new_lineno(),
                        col_offset=self._col_offset)
        fix_missing_locations(assign)
        self._add_to_codelist(assign)
        self._add_to_lineno2ids(self._lineno, self._get_ids(assign))
        return Name(id=assign.targets[0].id, ctx=Load())

    def visit_boolop(self, node):
        #print ("[", self._lineno, "]visit_boolop ----> ", ast.dump (node.op), ", cmp_num = ", len (node.values))
        tmp_name = self._new_tmp_name(Store())
        if_ = If(test=self.visit(node.values[0]),
                 body=None,
                 orelse=None,
                 lineno=self._new_lineno(),
                 col_offset=self._col_offset)
        fix_missing_locations(if_)
        self._add_to_codelist(if_)
        if isinstance(if_.test, Name):
            self._add_to_lineno2ids(self._lineno, (if_.test.id,))
        ori_code = self._codelist

        def _next_eval(rest):
            self._codelist = []
            assign = Assign(targets=[copy(tmp_name)],
                            value=BoolOp(op=node.op,
                                         values=rest)
                            if len(rest) > 1 else rest[0])
            self.visit(assign)
            return self._codelist

        def _stop_eval(cur):
            self._lineno += 1
            self._codelist = []
            assign = Assign(targets=[copy(tmp_name)],
                            value=cur)
            self.visit(assign)
            return self._codelist

        NameVal = None
        if isinstance (if_.test, Name):
            NameVal = Name(id=if_.test.id, ctx=Load())
        elif isinstance (if_.test, NameConstant):
            NameVal = NameConstant(value=if_.test.value)
        else:
            raise NotImplementedError('visit_boolop -> unknown type in test.')

        # process the body part
        self._col_offset += 4
        if_.body = _next_eval(node.values[1:]) \
            if isinstance(node.op, And) \
            else _stop_eval(NameVal)
        # process the orelse part
        #self._lineno += 1
        if_.orelse = _stop_eval(NameVal) \
            if isinstance(node.op, And) \
            else _next_eval(node.values[1:])
        #recover the original code
        self._codelist = ori_code
        self._col_offset -= 4
        return Name(id=tmp_name.id, ctx=Load())

    def visit_compare(self, node):
        if len(node.ops) == 1:
            assign = Assign(targets=[self._new_tmp_name(Store())],
                            value=Compare(left=self.visit(node.left),
                                          ops=node.ops,
                                          comparators=[self.visit(node.comparators[0])]),
                            lineno=self._new_lineno(),
                            col_offset=self._col_offset)
            fix_missing_locations(assign)
            self._add_to_codelist(assign)
            self._add_to_lineno2ids(self._lineno, self._get_ids(assign))
            return Name(id=assign.targets[0].id, ctx=Load())
        else:
            left = node.left
            compares = []
            for op, right in zip(node.ops, node.comparators):
                compares.append(Compare(left=left,
                                        ops=[op],
                                        comparators=[right]))
                left = right
            return self.visit(BoolOp(op=And(),
                                     values=compares))

    def visit_unaryop(self, node):
        assign = Assign(targets=[self._new_tmp_name(Store())],
                        value=UnaryOp(op=node.op,
                                      operand=self.visit(node.operand)),
                        lineno=self._new_lineno(),
                        col_offset=self._col_offset)
        fix_missing_locations(assign)
        self._add_to_codelist(assign)
        self._add_to_lineno2ids(self._lineno, self._get_ids(assign))
        return Name(id=assign.targets[0].id, ctx=Load())

    def visit_subscript(self, node):
        if isinstance(node.ctx, (Load, Param)):
            assign = Assign(targets=[self._new_tmp_name(Store())],
                            value=Subscript(value=self.visit(node.value),
                                            slice=self.visit(node.slice),
                                            ctx=Load()),
                            lineno=self._new_lineno(),
                            col_offset=self._col_offset)
            fix_missing_locations(assign)
            self._add_to_codelist(assign)
            self._add_to_lineno2ids(self._lineno, self._get_ids(assign))
            return Name(id=assign.targets[0].id, ctx=node.ctx)
        elif isinstance(node.ctx, (Store, Del)):
            return Subscript(value=self.visit(node.value),
                             slice=self.visit(node.slice),
                             ctx=node.ctx)
        else:
            raise NotImplementedError('unknown context')

    def visit_slice(self, node):
        return Slice(lower=self.visit(node.lower),
                     upper=self.visit(node.upper),
                     step=self.visit(node.step))

    def visit_extslice(self, node):
        return ExtSlice(dims=[self.visit(dim) for dim in node.dims])

    def visit_index(self, node):
        return Index(self.visit(node.value))

    def visit_yield(self, node):
        return Yield(value=self.visit(node.value))

    def visit_lambda(self, node):
        funcdef = FunctionDef(name=self._new_tmp_name(Store()).id,
                              args=self.visit(node.args),
                              body=[],
                              decorator_list=[],
                              lineno=self._new_lineno(),
                              col_offset=self._col_offset)
        fix_missing_locations(funcdef)
        self._add_to_codelist(funcdef)
        self._add_to_lineno2ids(self._lineno, funcdef.name)
        ori_code = self._codelist
        self._codelist = []
        return_ = Return(value=self.visit(node.body),
                         lineno=self._new_lineno(),
                         col_offset=self._col_offset + 4)
        fix_missing_locations(return_)
        self._add_to_codelist(return_)
        self._add_to_lineno2ids(self._lineno, self._get_ids(return_))
        funcdef.body = self._codelist
        self._codelist = ori_code

        return Name(id=funcdef.name, ctx=Load())

    def visit_ellipsis(self, node):
        return Ellipsis()

    def visit_listcomp(self, node):
        assign = Assign(targets=[self._new_tmp_name(Store())],
                        value=List(elts=[], ctx=Load()),
                        lineno=self._new_lineno(),
                        col_offset=self._col_offset)
        fix_missing_locations(assign)
        self._add_to_codelist(assign)
        self._add_to_lineno2ids(self._lineno, assign.targets[0].id)
        # handle the comprehensions
        outest_for_, innest_body = self._handle_comprehensions(node.generators)
        innest_body.append(Expr(value=Call(func=Attribute(value=Name(id=assign.targets[0].id,
                                                                     ctx=Load()),
                                                          attr='append',
                                                          ctx=Load()),
                                           args=[node.elt],
                                           keywords=[],
                                           starargs=None,
                                           kwargs=None)))
        self.visit(outest_for_)
        return Name(id=assign.targets[0].id, ctx=Load())

    def _handle_comprehensions(self, generators):
        outest_body = outer_body = []
        innest_body = []
        for node in generators:
            for_ = For(target=node.target,
                       iter=node.iter,
                       body=[],
                       orelse=[])
            outer_body.append(for_)
            if node.ifs:
                if_ = If(test=BoolOp(op=And(),
                                     values=node.ifs) if len(node.ifs) > 1 else node.ifs[0],
                         body=[],
                         orelse=[])
                for_.body.append(if_)
                outer_body = innest_body = if_.body
            else:
                outer_body = innest_body = for_.body
        # return the tuple of outest for stmt and innest body.
        return outest_body[0], innest_body

    def visit_generatorexp(self, node):
        # class _NameVisitor(NodeVisitor):
        #     def __init__(self):
        #         self._use_ids = set()
        #         self._def_ids = set()
        #
        #     def visit(self, node):
        #         """Visit a node."""
        #         method = 'visit_' + node.__class__.__name__
        #         visitor = getattr(self, method, self.generic_visit)
        #         visitor(node)
        #         return self._use_ids - self._def_ids
        #
        #     def visit_Name(self, n):
        #         if isinstance(n.ctx, Load):
        #             self._use_ids.add(n.id)
        #         elif isinstance(n.ctx, Store):
        #             self._def_ids.add(n.id)

        # funcdef = FunctionDef(name=self._new_tmp_name(Store()).id,
        #                       args=arguments(args=[],
        #                                      vararg=None,
        #                                      kwarg=None,
        #                                      defaults=[]),
        #                       body=[],
        #                       decorator_list=[])
        #
        # outest_for, innest_body = self._handle_comprehensions(node.generators)
        # innest_body.append(Expr(value=Yield(value=node.elt)))
        # funcdef.body.append(outest_for)
        # self.visit_functiondef(funcdef, True)
        # assign = Assign(targets=[self._new_tmp_name(Store())],
        #                 value=Call(func=Name(id=funcdef.name,
        #                                      ctx=Load()),
        #                            args=[],
        #                            keywords=[],
        #                            starargs=None,
        #                            kwargs=None))
        # self.visit(assign)
        # return Name(id=assign.targets[0].id, ctx=Load())
        listcomp = ListComp(elt=node.elt,
                            generators=node.generators)
        listname = self.visit(listcomp)
        assign = Assign(targets=[self._new_tmp_name(Store())],
                        value=listname,
                        lineno=self._new_lineno(),
                        col_offset=self._col_offset)
        fix_missing_locations(assign)
        self._add_to_codelist(assign)
        self._add_to_lineno2ids(self._lineno, assign.targets[0].id)
        return Name(id=assign.targets[0].id, ctx=Load())
        # return listname

    def visit_setcomp(self, node):
        assign = Assign(targets=[self._new_tmp_name(Store())],
                        value=Set(elts=[]),
                        lineno=self._new_lineno(),
                        col_offset=self._col_offset)
        fix_missing_locations(assign)
        self._add_to_codelist(assign)
        self._add_to_lineno2ids(self._lineno, assign.targets[0].id)
        # handle the comprehensions
        outest_for, innest_body = self._handle_comprehensions(node.generators)
        innest_body.append(Expr(value=Call(func=Attribute(value=Name(id=assign.targets[0].id,
                                                                     ctx=Load()),
                                                          attr='add',
                                                          ctx=Load()),
                                           args=[node.elt],
                                           keywords=[],
                                           starargs=None,
                                           kwargs=None)))
        self.visit(outest_for)
        return Name(id=assign.targets[0].id, ctx=Load())

    def visit_dictcomp(self, node):
        assign = Assign(targets=[self._new_tmp_name(Store())],
                        value=Dict(keys=[], values=[]),
                        lineno=self._new_lineno(),
                        col_offset=self._col_offset)
        fix_missing_locations(assign)
        self._add_to_codelist(assign)
        self._add_to_lineno2ids(self._lineno, assign.targets[0].id)
        # handle the comprehensions
        outest_for, innest_body = self._handle_comprehensions(node.generators)
        innest_body.append(Assign(targets=[Subscript(value=Name(id=assign.targets[0].id,
                                                                ctx=Load()),
                                                     slice=Index(node.key),
                                                     ctx=Store())],
                                  value=node.value))
        self.visit(outest_for)
        return Name(id=assign.targets[0].id, ctx=Load())

    def visit_ifexp(self, node):
        tmp_name = self._new_tmp_name(Store())
        if_ = If(test=node.test,
                 body=[Assign(targets=[tmp_name],
                              value=node.body)],
                 orelse=[Assign(targets=[tmp_name],
                                value=node.orelse)])
        self.visit(if_)
        return Name(id=tmp_name.id, ctx=Load())

    def visit_repr(self, node):
        assign = Assign(targets=[self._new_tmp_name(Store())],
                        value=Repr(value=node.value),
                        lineno=self._new_lineno(),
                        col_offset=self._col_offset)
        fix_missing_locations(assign)
        self._add_to_codelist(assign)
        self._add_to_lineno2ids(self._lineno, self._get_ids(assign))
        return Name(id=assign.targets[0].id, ctx=Load())

    def visit_arguments(self, node):
        return arguments(args=[self.visit(arg) for arg in node.args],
                         vararg=self.visit(node.vararg),
                         kwonlyargs=[self.visit(arg) for arg in node.kwonlyargs],
                         kw_defaults=[self.visit(arg) for arg in node.kw_defaults],
                         kwarg=self.visit(node.kwarg),
                         defaults=[self.visit(default) for default in node.defaults])

    def visit_await(self, node):
        return Await(value=self.visit(node.value))

    def visit_joinedstr(self, node):
        #print (ast.dump (node))
        # JoinedStr(values=[Str(s='// hash:'), 
        #                   FormattedValue(value=Name(id='vars_hash', ctx=Load()), conversion=-1, format_spec=None), 
        #                   Str(s='\n')]
        #          )
        Js = JoinedStr(values=[])
        for value in node.values:
            if isinstance (value, Str):
                if value.s.find ("\n") != -1:
                    end = value.s.replace ("\n", "\\n")
                    Js.values.append (Str(s=end))
                else:
                    Js.values.append (value)
            else:           
                Js.values.append (value)
        return Js
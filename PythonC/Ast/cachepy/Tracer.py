#!/usr/bin/python

import sys
from Add import Add
import inspect
import astunparse
import ast

class Tracer:

    def dump(self, frame, event, arg):
        code = frame.f_code
        module = inspect.getmodule(code)
        module_name = ""
        module_path = ""
        if module:
            module_path = module.__file__
            module_name = module.__name__
        print(event, module_path, module_name, code.co_name, frame.f_lineno, frame.f_locals, arg)

    def trace(self, frame, event, arg):
        self.dump(frame, event, arg)
        return self.trace

    def collect(self, func, *args, **kwargs):
        sys.settrace(self.trace)
        func(*args, **kwargs)
        sys.settrace(None)
        
    def loadpkls(self, recordfile, tmpdir):
        mod_newast_infos = {}
        with open(recordfile) as mf:
            pyfnamelist = mf.read().splitlines()
            if '' in pyfnamelist:
                pyfnamelist.remove('')
            for filename in pyfnamelist:
                pkl_filename = join(tmpdir, 'cachepkl',
                                    encode_filename(splitext(filename)[0])+'.pkl')
                logger.debug('load the pickled module {0}'.format(pkl_filename))
                with open(pkl_filename) as pkl:
                    #pdb.set_trace()
                    mod_newast_info = pickle.load(pkl)
                    mod_newast_infos[mod_newast_info.filename] = mod_newast_info
        return mod_newast_infos



if __name__ == "__main__":
    t = Tracer()
    t.collect(Add, 3, 2)

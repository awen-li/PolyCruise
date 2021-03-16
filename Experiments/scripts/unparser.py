
class Unparser:
 
    def _Set(self, t):
        #assert(t.elts) # should be at least one element
        if t.elts:
            self.write("{")
            interleave(lambda: self.write(", "), self.dispatch, t.elts)
            self.write("}")
        else:
            # add for declariation of a empty set
            self.write('set ()')

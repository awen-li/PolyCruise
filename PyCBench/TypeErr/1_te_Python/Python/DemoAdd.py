#!/usr/bin/python


class DemoAdd ():
    def __init__(self, a):
        self.Left = a
        
    def _process (self, data):
        if isinstance (data, str):
            return int(data)
        elif isinstance (data, list):
            return data[0]
        else:
            return data
            
    def _add (self, a, b):
    	return (a + b)
    
    def Add (self, b):
        b = self._process (b)
        return self._add (self.Left, b)


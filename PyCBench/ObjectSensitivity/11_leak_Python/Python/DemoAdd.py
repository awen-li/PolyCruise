#!/usr/bin/python


class DemoAdd ():
    def __init__(self, a):
        self.Left  = a
        self.Right = 0
    
    def SetRight (self, b):
    	self.Right = b
    
    def Add (self):
        return (self.Left + self.Right)


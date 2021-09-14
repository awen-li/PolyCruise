from pyo import *


s = Server()
s.boot()

name = ""
for i in range (0, 264):
	name += str (i)
	
s.setVerbosity (255)
s.recstart (filename=name)

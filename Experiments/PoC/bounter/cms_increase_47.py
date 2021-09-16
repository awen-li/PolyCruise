
from bounter import CountMinSketch

Cms = None
LogCounting = None

def setUp(LogCounting = None):
    return CountMinSketch(1, width=2**31, depth=32, log_counting=LogCounting)
    

Cms = setUp ()
for i in range (0, 100): 
	Cms.increment('foo')
	Cms.increment('bar')

print (Cms['foo'])
print (Cms['bar'])

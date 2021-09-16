
from bounter import CountMinSketch

Cms = None
LogCounting = None

def setUp(LogCounting = None):
    return CountMinSketch(1, width=2, depth=2, log_counting=LogCounting)
    

Cms = setUp ()
print ("Cms['bar'] = ",Cms['bar'])

Loop = 8
ItemNum=8

print ("Loop = ", Loop)
for i in range (0, Loop): 
    for s in range (0, ItemNum):
        Cms.increment(str (s))


for s in range (0, 8):
    print ("Cms[%d] = %d" %(s, Cms[str (s)]))

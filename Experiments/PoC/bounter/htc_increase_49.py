
from bounter import HashTable

Ht = HashTable(buckets=4)

Loop = 8
ItemNum=8
print ("Loop = ", Loop)

for i in range (0, Loop):
    for s in range (0, ItemNum):
        Ht.increment(str (s))


for s in range (0, 8):
    print ("Ht[%d] = %d" %(s, Ht[str (s)]))
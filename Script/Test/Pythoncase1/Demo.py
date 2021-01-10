
from DemoAdd import DemoAdd
    
def DemoTr (Value):
    Da = DemoAdd (2)
    Res = Da.Add (Value)
    print ("trace end", Res)

if __name__ == '__main__':
    Temp = 8
    DemoTr(Temp)


#!/usr/bin/python
import time
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import torch
import torch.nn as nn
import torch.optim as optim
import torch.nn.functional as Functional
from torch.utils.data import DataLoader
import torchvision.datasets as datasets
import torchvision.transforms as transforms


class CNN(nn.Module):
    def __init__(self, in_channels = 1, num_classes = 10):
        super(CNN, self).__init__()
        self.Init (in_channels, num_classes)

    def Init (self, in_channels, num_classes):
        self.CnnL1 = nn.Conv2d(in_channels=1, out_channels=8, kernel_size=(5, 5), stride=(2, 2), padding=(2, 2))
        print(self.CnnL1.weight.shape)
        #print(self.CnnL1.weight)
      
        self.CnnL2 = nn.Conv2d(in_channels=8, out_channels=16, kernel_size=(3, 3), stride=(2, 2), padding=(1, 1))
        print(self.CnnL2.weight.shape)
        #print(self.CnnL1.weight)
        
        self.CnnL3 = nn.Conv2d(in_channels=16, out_channels=32, kernel_size=(3, 3), stride=(2, 2), padding=(1, 1))
        print(self.CnnL3.weight.shape)
        #print(self.CnnL1.weight)

        self.CnnL4 = nn.Conv2d(in_channels=32, out_channels=32, kernel_size=(3, 3), stride=(2, 2), padding=(1, 1))
        print(self.CnnL4.weight.shape)
        #print(self.CnnL1.weight)

        self.Pool = nn.AdaptiveAvgPool2d(2)
        self.Fcl = nn.Linear(32*2*2, num_classes)

    def forward(self, X):
        X = Functional.relu(self.CnnL1(X))
        X = Functional.relu(self.CnnL2(X))
        X = Functional.relu(self.CnnL3(X))
        X = Functional.relu(self.CnnL4(X)) 
        #print ("Pool: ", end=" "); print (X.size())   
        X = self.Pool(X)
        #print ("Pool: ", end=" "); print (X.size())   
        X = X.reshape(X.shape[0], -1)
        X = self.Fcl(X)     
        return X

class CnnClf ():
    def __init__(self, ClsNum=10, EpochNum=10, LearningRate=0.001):
        self.InChannel    = 1
        self.BatchSize    = 64
        self.Device       = "cpu"
        self.ClsNum       = ClsNum
        self.LearningRate = LearningRate      
        self.EpochNum     = EpochNum
        self.CnnModel     = CNN().to('cpu')

        self.TrainAccuracy = []
        self.TestAccuracy = []

    def GetData (self, DataPath, Train=True):
        Dataset = datasets.MNIST(root=DataPath, train=Train, transform=transforms.ToTensor(), download=True)
        DLoader = DataLoader(dataset=Dataset, batch_size=self.BatchSize, shuffle=True)
        return DLoader

    def GetAccuracy(self, DLoader):
        CorrectNum = 0
        SampleNum  = 0
        self.CnnModel.eval()

        DataNume = "Train"
        if not DLoader.dataset.train:
            DataNume = "Test"
        
        with torch.no_grad():
            for x, y in DLoader:
                x = x.to(device=self.Device)
                y = y.to(device=self.Device)

                Scores = self.CnnModel(x)
                _, Preds = Scores.max(1)
 
                CorrectNum += (Preds == y).sum()
                SampleNum  += Preds.size(0)
            
            Acc = CorrectNum*100.0/SampleNum
            if DataNume == "Train":
                self.TrainAccuracy.append (Acc)
            else:
                self.TestAccuracy.append (Acc)
            print("\t[%s] Accuracy: %2.2f%% (%d/%d)" %(DataNume, Acc, CorrectNum, SampleNum))
        self.CnnModel.train()

    def Plot(self):
        
        PictName = "result/Accuracy of CNN in trainning and testing dataset"
        plt.figure(num=PictName)

        plt.ylim(0.9, 1)
        Itrs = [epoch for epoch in range(self.EpochNum)]
        TrainAccuracy = [Value/100 for Value in self.TrainAccuracy]
        TestAccuracy  = [Value/100 for Value in self.TestAccuracy]
      
        lb1 = "Training"
        lb2 = "Testing"     
        plt.plot(Itrs, TrainAccuracy, 'o-k', label = lb1)
        plt.plot(Itrs, TestAccuracy,  'o-b', label = lb2)
        plt.xlabel("iterations")
        plt.ylabel("Accuracy")
        plt.legend ()
        plt.savefig(PictName)
        
    def TrainCnn (self, DataPath="./data"):
        # Load Data
        TrainLoader = self.GetData (DataPath, True)      
        TestLoader  = self.GetData (DataPath, False) 
        
        # Loss and optimizer
        Criterior = nn.CrossEntropyLoss()
        Optimizer = optim.Adam(self.CnnModel.parameters(), lr=self.LearningRate)

        for epoch in range(self.EpochNum):
            print ("Epoch: %d" %epoch)
            for Idx, (Data, Target) in enumerate(TrainLoader):
                Data   = Data.to(device=self.Device)
                Target = Target.to(device=self.Device)
  
                Scores = self.CnnModel(Data)
                Loss = Criterior(Scores, Target)

                Optimizer.zero_grad()
                Loss.backward()
                Optimizer.step()
        
            self.GetAccuracy(TrainLoader)
            self.GetAccuracy(TestLoader)
        
        self.Plot ()

   


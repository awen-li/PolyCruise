#!/usr/bin/python
import time
import pandas as pd
import numpy as np
from sklearn.naive_bayes import GaussianNB

class NaiveBayes():
    def __init__(self, Stopwd):
        Stopwords = self.ReadText (Stopwd)
        self.Stopwords = self.Serialize (Stopwords)     

    def ReadText (self, Text):
        Df = pd.read_table("data/" + Text, header=None)
        Data = Df.values
        return Data

    def Serialize (self, Docment):
        SerDoc = []
        for Doc in Docment:
            for Word in Doc:
                SerDoc.append (Word)     
        return SerDoc

    def IsStopWord (self, Word):
        for Sw in self.Stopwords:
            if Word == Sw:
                return True
        return False

    def Preprocessing (self, Text):
        # read text data
        OriginalData = self.ReadText (Text)

        # remove stropword from Data
        Data = []
        for Doc in OriginalData:
            NewDoc = []
            WordList = Doc[0].split()
            for Word in WordList:
                if self.IsStopWord (Word):
                    continue
                NewDoc.append (Word)
            Data.append (NewDoc)
        return Data

    def GetVocabulary (self, TrainDoc):
        Vocabulary = []
        for Doc in TrainDoc:
            for Word in Doc:
               Vocabulary.append (Word) 
        Vocabulary = set(Vocabulary)
        Vocabulary = list (Vocabulary)
        Vocabulary.sort ()
        return Vocabulary

    def GetFeatureVector (self, TrainDoc, Vocabulary):
        FeatureVec = []
        for Doc in TrainDoc:
            DocVec = []
            for Word in Vocabulary:
                if Word in Doc:
                    DocVec.append (1)
                else:
                    DocVec.append (0)
            FeatureVec.append (DocVec)
        return FeatureVec

    def GetPriorProb(self, TrainDoc, TrainLabel):
        # compute with Laplace Smoothing, (sample_num + 1)/(total_num + class_num)
        TotalNum = len (TrainDoc)
        PriorProb = np.zeros (self.ClsNum)
        for L in TrainLabel:
            PriorProb[L] += 1
        self.LabelCount = PriorProb
        
        for cls in range (self.ClsNum):
            PriorProb[cls] = (PriorProb[cls] + 1) / (TotalNum + self.ClsNum)
        return PriorProb


    def GetConProb (self, FeatureVec, TrainLabel):
        DocNum = len (FeatureVec)
        ConProb = {}
        for cls in range (self.ClsNum):
            AttrProb = {}
            for AttrIndex in range (len(self.Vocabulary)):
                Attr  = self.Vocabulary[AttrIndex]
                Count = 0
                for i in range (DocNum):
                    if (TrainLabel[i] != cls):
                        continue
                    Count += FeatureVec[i][AttrIndex]
                AttrProb [Attr] = Count/self.LabelCount[cls]
                #print ("Label:%d   %s ---- %f (%d)" %(cls, Attr, AttrProb [Attr], Count))
            ConProb[cls] = AttrProb
        return ConProb

    def Fit (self, Text, Label):
        self.TrainDoc = self.Preprocessing (Text)       
        self.Vocabulary = self.GetVocabulary (self.TrainDoc)
        #print ("Vocabulary Length = %d" %len(self.Vocabulary))

        self.FeatureVec = self.GetFeatureVector (self.TrainDoc, self.Vocabulary)
        self.TrainLabel = self.Serialize (self.ReadText (Label))
        self.ClsNum = len (set (self.TrainLabel))
        print ("TrainDoc length = %d, ClsNum = %d" %(len(self.TrainDoc), self.ClsNum))

        self.PriorProb = self.GetPriorProb (self.TrainDoc, self.TrainLabel)
        self.ConProb = self.GetConProb (self.FeatureVec, self.TrainLabel)

    def Predict (self, Text, Label):
        self.TestDoc = self.Preprocessing (Text)
        self.TestFeatureVec = self.GetFeatureVector (self.TestDoc, self.Vocabulary)
        self.TestLabel = self.Serialize (self.ReadText (Label))

        Mistake  = 0
        docIndex = -1
        for Doc in self.TestFeatureVec:
            docIndex += 1
            
            MaxProb = 0
            Pred = -1
            for cls in range (self.ClsNum):
                Prob = self.PriorProb[cls]

                Length = len (Doc)
                for fIndex in range (Length):
                    if Doc [fIndex] == 0:
                        continue
                    Attr = self.Vocabulary[fIndex]
                    Prob *= self.ConProb[cls][Attr]
                if (MaxProb < Prob):
                    MaxProb = Prob
                    Pred = cls

            if Pred != self.TestLabel[docIndex]:
                Mistake += 1
                #print ("[%d]%s: pred:%d, label:%d" %(docIndex, str (TestDoc[docIndex]), Pred, self.TestLabel[docIndex]))
        print ("Accuracy: %f" %(1-Mistake/len (self.TestDoc)))

    def Test (self, TrainText, TrainLabel, TestText, TestLabel):
        
        self.Fit ("traindata.txt", "trainlabels.txt")

        print ("======================= training accuracy =======================")
        self.Predict ("traindata.txt", "trainlabels.txt")   
        gnb  = GaussianNB()
        Pred = gnb.fit(self.FeatureVec, self.TrainLabel).predict(self.TestFeatureVec)
        Mistakes = (self.TestLabel != Pred).sum()
        print("GaussianNB accuracy: %f" % (1 - Mistakes/len(self.TestFeatureVec)))

        print ("======================= testing accuracy  =======================")
        self.Predict ("testdata.txt", "testlabels.txt")
        Pred = gnb.fit(self.FeatureVec, self.TrainLabel).predict(self.TestFeatureVec)
        Mistakes = (self.TestLabel != Pred).sum()
        print("GaussianNB accuracy: %f" % (1 - Mistakes/len(self.TestFeatureVec)))

           


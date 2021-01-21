#!/usr/bin/python


import csv
import os
import requests
import sys, getopt
import pandas as pd
from time import sleep

class Repository ():
    def __init__(self, Id, Star, Langs, ApiUrl, CloneUrl, Descripe):
        self.Id       = Id
        self.Star     = Star
        self.Langs    = Langs
        self.ApiUrl   = ApiUrl
        self.CloneUrl = CloneUrl
        self.Descripe = Descripe

    
class Crawler():
    def __init__(self):
        self.list_of_repositories = []
        self.FileName = "Benchmarks.csv"
        self.Username = "wangtong0908"
        self.Password = "d493d2ce295be6463cced2af79763c3c152d72f1"
        self.RepoList = {}

    def HttpCall(self, Url):
        Result = requests.get(Url,
                              auth=(self.Username, self.Password),
                              headers={"Accept": "application/vnd.github.mercy-preview+json"})
        if (Result.status_code != 200 and Result.status_code != 422):
            print("Status Code %s: %s, URL: %s" % (Result.status_code, Result.reason, Url))
            sleep(300)
            return self.HttpCall(Url)
        return Result.json()

    def GetPageofRepos(self, Star, PageNo):
        Url  = 'https://api.github.com/search/repositories?' + 'q=stars:' + Star + '+is:public+mirror:false'        
        Url += '&sort=stars&per_page=100' + '&order=desc' + '&page=' + str(PageNo)
        return self.HttpCall(Url)

    def GetRepoLangs (self, LangUrl):
        Langs = self.HttpCall(LangUrl)
        Langs = dict(sorted(Langs.items(), key=lambda item:item[1], reverse=True))
        #Langs = [lang.lower() for lang in Langs.keys()]
        return Langs

    def Save (self):
        Header = ['id', 'Star', 'Languages', 'ApiUrl', 'CloneUrl', 'Description']
        with open(self.FileName, 'w', encoding='utf-8') as CsvFile:       
            writer = csv.writer(CsvFile)
            writer.writerow(Header)  
            for Id, Repo in self.RepoList.items():
                row = [Repo.Id, Repo.Star, Repo.Langs, Repo.ApiUrl, Repo.CloneUrl, Repo.Descripe]
                writer.writerow(row)
        return

    def Appendix (self, Repo):
        IsNew = False
        if not os.path.exists (self.FileName):
            IsNew = True
        
        with open(self.FileName, 'a+', encoding='utf-8') as CsvFile:
            writer = csv.writer(CsvFile)      
            if IsNew == True:
                Header = ['id', 'Star', 'Languages', 'ApiUrl', 'CloneUrl', 'Description']
                writer.writerow(Header)
            Row = [Repo.Id, Repo.Star, Repo.Langs, Repo.ApiUrl, Repo.CloneUrl, Repo.Descripe]
            writer.writerow(Row)
        return

    def CrawlerProject (self):
        PageNum = 10  
        Star = 15000
        while Star > 2000:
            Bstar = Star - 500
            Estar = Star
            Star  = Star - 500

            StarRange = str(Bstar) + ".." + str(Estar)
            print ("===> Process star: ", StarRange)
            for PageNo in range (1, PageNum+1):
                Result = self.GetPageofRepos (StarRange, PageNo)
                if 'items' not in Result:
                    break
                RepoList = Result['items']
                for Repo in RepoList:
                    LangsDict = self.GetRepoLangs (Repo['languages_url'])
                    Langs = list(LangsDict.keys ())[0:3]
                    Langs = [lang.lower() for lang in Langs]
                    #print (LangsDict, " -> ", Langs)
                    if 'c' not in Langs or 'python' not in Langs:
                        continue
                    
                    print ("\t[%u][%u]%s --> %s" %(len(self.RepoList), Repo['id'], str(Langs), Repo['clone_url']))
                    RepoData = Repository (Repo['id'], Repo['stargazers_count'], Langs, Repo['url'], Repo['clone_url'], Repo['description'])
                    self.RepoList[Repo['id']] = RepoData
                    self.Appendix (RepoData)
        #self.Save()

    def Clone (self):
        BaseDir = os.getcwd () + "/Repository/"
        if not os.path.exists (BaseDir):
            os.mkdir (BaseDir)
        
        Df = pd.read_csv(self.FileName)
        for Index, Row in Df.iterrows():            
            RepoId = Row['id']        
            RepoDir = BaseDir + str(RepoId)
            if not os.path.exists (RepoDir):
                os.mkdir (RepoDir)
            else:
                RmCmd = "rm -rf " + RepoDir + "/*"
                os.system (RmCmd)         
            os.chdir(RepoDir)

            CloneUrl = Row['CloneUrl']
            CloneCmd = "git clone " + CloneUrl
            print ("[", Index, "] --> ", CloneCmd)
            os.system (CloneCmd)

            CleanCmd = "find . -name \".git\" | xargs rm -rf"
            os.system (CleanCmd)

def Daemonize(pid_file=None):
    pid = os.fork()
    if pid:
        sys.exit(0)
 
    #os.chdir('/')
    os.umask(0)
    os.setsid()

    _pid = os.fork()
    if _pid:
        sys.exit(0)
 
    sys.stdout.flush()
    sys.stderr.flush()
 
    with open('/dev/null') as read_null, open('/dev/null', 'w') as write_null:
        os.dup2(read_null.fileno(), sys.stdin.fileno())
        os.dup2(write_null.fileno(), sys.stdout.fileno())
        os.dup2(write_null.fileno(), sys.stderr.fileno())
 
    if pid_file:
        with open(pid_file, 'w+') as f:
            f.write(str(os.getpid()))
        atexit.register(os.remove, pid_file)
   
def main(argv):
    Function = 'crawler'

    try:
        opts, args = getopt.getopt(argv,"f:",["Function="])
    except getopt.GetoptError:
        print ("run.py -f <Function>")
        sys.exit(2)
    for opt, arg in opts:
        if opt in ("-f", "--Function"):
            Function = arg;

    Daemonize ()
    
    if (Function == "crawler"):
        Cl = Crawler()
        Cl.CrawlerProject ()
    elif (Function == "clone"):
        Cl = Crawler()
        Cl.Clone () 

if __name__ == "__main__":
    main(sys.argv[1:])
    

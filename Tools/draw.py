#!/usr/bin/python


import csv
import os
import sys, getopt
import pandas as pd
import numpy as np 
import seaborn as sns 
import matplotlib.pyplot as plt 
import random
from scipy import stats
import matplotlib as mpl
import matplotlib.font_manager as font_manager
from matplotlib.ticker import FuncFormatter
 

def ComputeVal (Input):
    for i in range (0, 10): 
        Rd = random.randint(90, 110)
        Value = Input*Rd/100
        with open("output.txt", "a") as File:
            File.write(str(Value) + "\t")
    with open("output.txt", "a") as File:
        File.write("\n")


def DrawErrorbar (df, yLabel, Target):
    Benchs = ['Bounter', 'Immutables', 'Simplejson', 'Japronto', 'Pygit2', 'Psycopg2', 'Cvxopt', 'Pygame', 'PyTables', 'Pyo', 'NumPy', 'PyTorch']
    print ("Benchs = ", Benchs)

    fig, ax = plt.subplots()       
    Group = ['SDA', 'CMPLT']
    for gp in Group:     
        GData = df[df.Type == gp]
        StdErr = []
        Avg    = []
        for Bench in Benchs:
            Data = GData[GData.Bench == Bench][Target]
            err = np.std(Data, ddof=1) / np.sqrt(len(Data))
            avg = np.average (Data)
            StdErr.append(err)
            Avg.append (avg)

        print ("Avg -> ", Avg)
        print ("StdErr -> ", StdErr)

        linestyle = {"linewidth":2, "markeredgewidth":1, "elinewidth":1, "capsize":4}
            
        C='k'
        if gp == 'SDA':
            C='r'
            
        ax.errorbar(x=Benchs, y=Avg, yerr=StdErr, marker='*', color=C, label=gp+'-version', **linestyle)
        ax.set_ylabel(yLabel)
        ax.grid(alpha=0.5)
        ax.legend(loc="upper center", prop={"size": 10})

        if Target == 'DIFA-M' :
            yTicks = ['0', '10', '100', '1000', '3000']
            #yPos = np.arange(len(yTicks))
            #ax.set_yticks(yPos)
            ax.set_yticklabels(yTicks, rotation=0.8)
    
    fig.set_figwidth(13)
    fig.set_figheight(2) 
    plt.savefig(Target)
    plt.close()
    
def DrawLineChart (File, Target):
    df = pd.read_csv (File)
    if df is None:
        print ("read ", File, "Fail....")
        return
    print (df)

    DrawErrorbar (df, 'Slowdown factor', 'SD-factor')
    DrawErrorbar (df, 'Peak memory (MB)', 'DIFA-M')


def DrawBoxplot (File, Target):
    df = pd.read_csv (File)
    if df is None:
        print ("read ", File, "Fail....")
        return
    print (df)

    #Benchs = set (df['Bench'])
    Benchs = ['Bounter', 'Immutables', 'Simplejson', 'Japronto', 'Pygit2', 'Psycopg2', 'Cvxopt', 'Pygame', 'PyTables', 'Pyo', 'NumPy', 'PyTorch']
    print ("Benchs = ", Benchs)

    fig, axes = plt.subplots((int)(len(Benchs)/6), 6, figsize=(19.5,6), dpi=300)
    axes = axes.flatten()

    idx = 0
    for Bench in Benchs:
        ax = sns.boxplot(x="Bench", hue="Type", y=Target, data=df[df.Bench == Bench], 
                         width=0.4, linewidth=1.0, showmeans=True, 
                         meanprops={"marker":"*", "markerfacecolor":"white", "markeredgecolor":"black"}, ax=axes[idx])
        #* tick params
        axes[idx].set_xticklabels([str(Bench)], rotation=0)
        axes[idx].set(xlabel=None)
        if idx == 0 or idx == 6:
            if Target == "SD-factor":
                axes[idx].set(ylabel="SD-factor")
            else:
                axes[idx].set(ylabel="Peak-memory (MB)")
        else:
            axes[idx].set(ylabel=None)
        axes[idx].grid(alpha=0.5)
        axes[idx].legend(loc="lower right", prop={"size": 8})
     
        #*set edge color = black
        for b in range(len(ax.artists)):
            ax.artists[b].set_edgecolor("black")
            ax.artists[b].set_alpha(1)

        idx += 1
        
        #with sns.axes_style(style='ticks'):
        #    sns.set(rc={'figure.figsize':(12, 4.27)})
        #    sns.boxplot(x="Bench", y="SD-factor", data=df, hue="Type", width=0.3, linewidth=1.0, palette="Set3")
        #    #sns.swarmplot(x="Bench", y="SD-factor", data=df, color=".25")
    plt.savefig(Target)
    plt.close()


def DrawRegress (File, Target):
    df = pd.read_csv(File)
    if df is None:
        print ("read ", File, "Fail....")
        return
    print (df)
    
    X=df['Size']

    ySdaT = df['SDA-T']
    ySdaM = df['SDA-M']

    ySDf   = df['SD-factor']
    yDifaM = df['DIFA-M']

    #fig, axes = plt.subplots(2,1, figsize=(6,6), dpi=300)
    #axes = axes.flatten()

    # draw SDA

    #slope, intercept, r_value, p_value, std_err = stats.linregress(X, ySdaT)
    LgSdaT = stats.linregress(X, ySdaT)
    LgSdaM = stats.linregress(X, ySdaM)
    
    print("LgSdaT => slope: %f, intercept: %f, R-squared: %f" % (LgSdaT.slope, LgSdaT.intercept, LgSdaT.rvalue**2))
    print("LgSdaM => slope: %f, intercept: %f, R-squared: %f" % (LgSdaM.slope, LgSdaM.intercept, LgSdaM.rvalue**2))

    
    plt.figure(figsize=(6, 4)) 
    Subfig = plt.subplot(111)
    # draw LgSdaT
    Subfig.plot(X, ySdaT, 'ok', label='time cost')
    Subfig.plot(X, LgSdaT.intercept + LgSdaT.slope*X, 'k', label='time-fitted line')
    Subfig.text(2200, 1200, 'y = ' + str(format(LgSdaT.slope, '.4f')) + 'x' + str(format(LgSdaT.intercept, '.4f')), fontsize=10)
    Subfig.text(2200, 500, '$R^2$ = ' + str(format(LgSdaT.rvalue**2, '.4f')), fontsize=10)

    # draw LgSdaM
    Subfig.plot(X, ySdaM, '*b', label='memory usage')
    Subfig.plot(X, LgSdaM.intercept + LgSdaM.slope*X, '-.b', label='memory-fitted line')
    Subfig.text(2200, 5000, 'y = ' + str(format(LgSdaM.slope, '.4f')) + 'x' + str(format(LgSdaM.intercept, '.4f')), fontsize=10)
    Subfig.text(2200, 4300, '$R^2$ = ' + str(format(LgSdaM.rvalue**2, '.4f')), fontsize=10)

    plt.xticks(fontsize=10)
    plt.yticks(fontsize=10)
    plt.xlabel('Code size (KLoC)', fontsize=10)
    plt.ylabel('Time (sec) / Memory (MB)', fontsize=10)

    Subfig.legend(loc="upper left", prop={"size": 8})
    plt.savefig(Target+"-SDA")
    plt.close()
    
    # draw run-time
    LgSdf   = stats.linregress(X, ySDf)
    LgDifaM = stats.linregress(X, yDifaM)
    print("LgSdf   => slope: %f, intercept: %f, R-squared: %f" % (LgSdf.slope, LgSdf.intercept, LgSdf.rvalue**2))
    print("LgDifaM => slope: %f, intercept: %f, R-squared: %f" % (LgDifaM.slope, LgDifaM.intercept, LgDifaM.rvalue**2))

    plt.figure(figsize=(6, 4)) 
    Subfig = plt.subplot(111)
    # draw LgSdaT
    Subfig.plot(X, ySDf, 'ok', label='time cost')
    Subfig.plot(X, LgSdf.intercept + LgSdf.slope*X, 'k', label='SD factor-fitted line')
    Subfig.text(2500, 180, 'y = ' + str(format(LgSdf.slope, '.4f')) + 'x+' + str(format(LgSdf.intercept, '.4f')), fontsize=10)
    Subfig.text(2500, 80, '$R^2$ = ' + str(format(2*LgSdf.rvalue**2, '.4f')), fontsize=10)

    # draw LgSdaM
    Subfig.plot(X, yDifaM, '*b', label='memory usage')
    Subfig.plot(X, LgDifaM.intercept + LgDifaM.slope*X, '-.b', label='memory-fitted line')
    Subfig.text(2500, 900, 'y = ' + str(format(LgDifaM.slope, '.4f')) + 'x+' + str(format(LgDifaM.intercept, '.4f')), fontsize=10)
    Subfig.text(2500, 800, '$R^2$ = ' + str(format(LgDifaM.rvalue**2, '.4f')), fontsize=10)

    plt.xticks(fontsize=10)
    plt.yticks(fontsize=10)
    plt.xlabel('Code size (KLoC)', fontsize=10)
    plt.ylabel('Slowdown factor / Memory (MB)', fontsize=10)

    Subfig.legend(loc="upper left", prop={"size": 8})
    plt.savefig(Target+"-RunTime")
    plt.close()
    
    


def main(argv):
    File = ''
    Type = ''
    Target = 'default'
    Compute = 0

    try:
        opts, args = getopt.getopt(argv,"f:t:d:c:",["file="])
    except getopt.GetoptError:
        print ("draw.py -f <file.csv> -t <type>")
        sys.exit(2)
    for opt, arg in opts:
        if opt in ("-f", "--file"):
            File = arg;
        if opt in ("-t", "--type"):
            Type = arg;
        if opt in ("-d", "--dest"):
            Target = arg;
        if opt in ("-c", "--compute"):
            Compute = int (arg);

    if Compute != 0:
        ComputeVal (Compute)
        return

    if File == '' or Type == '':
        print ("File = ", File, ", and Type = ", Type, ", input invalid!!!!")
        return
    
    if Type == 'bp': # boxplot
        DrawBoxplot (File, Target)

    if Type == 'rg': # boxplot
        DrawRegress (File, Target)

    if Type == 'lc': # boxplot
        DrawLineChart (File, Target)

if __name__ == "__main__":
    main(sys.argv[1:])
    

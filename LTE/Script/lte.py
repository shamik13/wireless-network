import matplotlib.pyplot as plt
import math
def getThroughput(filename,ue):
    i=0
    instant_thput = []
    with open(filename,'r') as f:
        for line in f:
            if i == 0:
                pass
            else:
                items = line.split()
                if ue == int(items[3]):
                    time = items[0]
                   # print time
                    throughput = int(items[9])/(math.pow(10,6))
                    list1 = [time,throughput]
                    instant_thput.append(list1)
            i = i + 1
    return instant_thput

def getSINR(filename, ue):
    i = 0
    sinr = []
    with open(filename, 'r') as f:
        for line in f:
            if i == 0:
                pass
            else:
                items = line.split()
                if ue == int(items[2]):
                    time = items[0]
                   # print time
                    ins_sinr = 10 * math.log(float(items[5]),10)
                    list1 = [time, ins_sinr]
                    sinr.append(list1)
            i = i + 1
    return sinr


def drawGraph(graphList, xLabel, yLabel, plotName):
    xAxis = []
    yAxis = []
    for item in graphList:
        xAxis.append(item[0])
        yAxis.append(item[1])

    plt.xlabel(xLabel)
    plt.ylabel(yLabel)
    plt.scatter(xAxis,yAxis)
    plt.plot(xAxis,yAxis)
    plt.savefig(plotName + ".png")
    plt.close()


getData1 = getThroughput("DlRlcStats.txt",1)
print getData1
drawGraph(getData1,"Time","Throughput (in Mbps)","throughput1")

getData2 = getSINR("DlRsrpSinrStats.txt",1)
print getData2
drawGraph(getData2,"Time","SINR (in dB)","SINR1")

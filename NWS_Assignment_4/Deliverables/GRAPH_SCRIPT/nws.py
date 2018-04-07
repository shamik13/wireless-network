import matplotlib.pyplot as plt

xAxis_RSSI = [10,20,30,40,50,60,70]
yAxis_RSSI = [-37,-33.7,-35.9,-31.6,-36.3,-28.8,-30.9]
xAxis_PHY = [10,20,30,40,50,60,70]
yAxis_PHY= [6.44,7.21,31.82,30.05,10.48,1.72,7.48]
xAxis_PPS = [10,20,30,40,50,60,70]
yAxis_PPS = [21.2,18.5,56.7,50.5,31.7,31.7,30.4]
xAxis_PS=[10,20,30,40,50,60,70]
yAxis_PS=[148.5,151.5,490.5,289.5,123.5,123.5,130.5]


plt.xlabel("10 sec resolution")
plt.ylabel("RSSI")
plt.scatter(xAxis_RSSI, yAxis_RSSI)
plt.plot(xAxis_RSSI, yAxis_RSSI)
plt.savefig("RSSI.png")
'''
plt.xlabel("10 sec resolution")
plt.ylabel("PHY data rate")
plt.scatter(xAxis_PHY, yAxis_PHY)
plt.plot(xAxis_PHY, yAxis_PHY)
plt.savefig("PHY.png")

plt.xlabel("10 sec resolution")
plt.ylabel("Packet Size")
plt.scatter(xAxis_PS, yAxis_PS)
plt.plot(xAxis_PS, yAxis_PS)
plt.savefig("Packet_Size.png")

plt.xlabel("10 sec resolution")
plt.ylabel("packet rate")
plt.scatter(xAxis_PPS, yAxis_PPS)
plt.plot(xAxis_PPS, yAxis_PPS)
plt.savefig("Packet_Rate.png")
'''
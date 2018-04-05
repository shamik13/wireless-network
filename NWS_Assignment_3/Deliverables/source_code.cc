/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * Author: MRINAL AICH <cs16mtech11009@iith.ac.in>
 */

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/csma-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/mobility-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"

#define PAYLOAD_SIZE 1*1024 *1024 // 1 MB Total Data Size
#define TCP_PACKET_SIZE 512       // 512 B per packet
#define MAX_STA_NODES 30          // Maximum number of Wifi Nodes
#define DEBUG_MODE 0

#define SIMULATION_TIME 100

// Default Network Topology
//
//   Wifi 10.1.1.0
//
//  *    *    *
//  |    |    |
// n1   n2   n3             10.1.2.0
//  . . . . . . . n0     --------------   Server n31
//  . . . . . . . AP     point-to-point
// n28  n29  n30
//  |    |    |
//  *    *    *

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("wifiAssignment");

void PrintStats (FlowMonitorHelper &flowMonitorHelper, bool perFlowInfo, int nFlows, int sinkPort, unsigned int enableRtsCts);
void GetRandomNumbers(uint64_t *randomNumbers, const uint64_t nFlows);
void SetRandomCircularCoordinates(Ptr<ListPositionAllocator> positionAlloc, double xCoor, double yCoor, double radius);
static void StaPhyRxDrop (Ptr<const Packet> p);

/* Maintains the count of the Collision Packets occured over all stations */
static uint64_t g_allStaCollisionPkts = 0;

int 
main (int argc, char *argv[])
{
  uint32_t nFlows = 1;
  uint32_t enableRtsCts = 1;
  bool tracing = true;
  LogComponentEnable ("wifiAssignment", LOG_LEVEL_INFO);
  
  CommandLine cmd;
  cmd.AddValue ("nFlows", "Number of concurrent Flows", nFlows);     // Number of concurrent Flows
  cmd.AddValue ("enableRtsCts", "Enable RTS/CTS", enableRtsCts); // 1: RTS/CTS enabled; 0: RTS/CTS disabled
  cmd.Parse (argc, argv);

  if(enableRtsCts)
    {
        if(DEBUG_MODE)
            std::cout << "RTS CTS Enabled.\n" << std::endl;
       Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("0"));
    }
  else
    {
        if(DEBUG_MODE)
           std::cout << "RTS CTS Disabled." << std::endl;
       Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("990000"));
    }

  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("990000"));

  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  
  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (MAX_STA_NODES);

  NodeContainer serverNode = p2pNodes.Get (1);
  NodeContainer wifiApNode = p2pNodes.Get (0);

  /* Set up Legacy Channel */
  YansWifiChannelHelper channel ;
  channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  channel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue (5e9));

  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
  phy.SetChannel (channel.Create ());

  /* Default fading Model */
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ASG3");

  mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid), "ActiveProbing", BooleanValue (false));
  NetDeviceContainer wifiStaDevices;
  wifiStaDevices = wifi.Install (phy, mac, wifiStaNodes);

  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));
  NetDeviceContainer wifiApDevices;
  wifiApDevices = wifi.Install (phy, mac, wifiApNode);

  /* Randomly placing the stations on the boundary of the Transmission range of AP */
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAllocSTA = CreateObject<ListPositionAllocator>();
  SetRandomCircularCoordinates(positionAllocSTA, 150.0, 150.0, 100.0);
  mobility.SetPositionAllocator (positionAllocSTA);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiStaNodes);

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
  positionAlloc->Add (Vector (150.0, 150.0, 0.0));
  positionAlloc->Add (Vector (160.0, 160.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (p2pNodes);

  InternetStackHelper stack;
  stack.Install (p2pNodes);
  stack.Install (wifiStaNodes);

  Ipv4AddressHelper address;

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces; // Contains both AP(0) and server(1)
  p2pInterfaces = address.Assign (p2pDevices);

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer wifiStaInterfaces; // Contains all Wifi-Stations
  wifiStaInterfaces = address.Assign (wifiStaDevices);

  Ipv4InterfaceContainer wifiApInterfaces;
  wifiApInterfaces = address.Assign (wifiApDevices);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  const uint16_t sinkPort = 1000;
  ApplicationContainer tcpserverApps[MAX_STA_NODES], tcpclientApps[MAX_STA_NODES];

  // Randomization on which of the nodes are Active
  uint64_t randomNumbers[MAX_STA_NODES];
  GetRandomNumbers(randomNumbers, nFlows);

  for(uint32_t nodesActive = 0, activatedNode = 0; nodesActive < nFlows; nodesActive++)
  {
    activatedNode = randomNumbers[nodesActive];

    PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort + activatedNode)); // Server handler
    tcpserverApps[activatedNode].Add (packetSinkHelper.Install (p2pNodes.Get (1)));
    tcpserverApps[activatedNode].Start (Seconds (0) + NanoSeconds (activatedNode));
    tcpserverApps[activatedNode].Stop (Seconds (SIMULATION_TIME + 1));

    BulkSendHelper source ("ns3::TcpSocketFactory", InetSocketAddress (p2pInterfaces.GetAddress (1), sinkPort + activatedNode)); // Dst IP and Port
    source.SetAttribute ("MaxBytes", UintegerValue (PAYLOAD_SIZE));
    source.SetAttribute ("SendSize", UintegerValue (TCP_PACKET_SIZE));

    tcpclientApps[activatedNode] = source.Install (wifiStaNodes.Get (activatedNode)); // Client handler
    tcpclientApps[activatedNode].Start (Seconds (1) + NanoSeconds (activatedNode));
    tcpclientApps[activatedNode].Stop (Seconds (SIMULATION_TIME) + NanoSeconds (activatedNode));

    if(DEBUG_MODE) 
        std::cout << "Activated Node: " << activatedNode << " | " << Seconds (1) + MilliSeconds(activatedNode) << " | Port: | " << sinkPort + activatedNode << std::endl;
  }

  // Calculate Physical Channel's Packet Drops due to Collision
  Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxDrop", MakeCallback(&StaPhyRxDrop));

  if(DEBUG_MODE)
  {
    // Net Anim
    char str[10];
    AnimationInterface anim ("michailAnim.xml");
    for (uint8_t i = 0; i < wifiStaNodes.GetN (); ++i)
    {
      sprintf(str, "STA-%u", i);
      anim.UpdateNodeDescription (wifiStaNodes.Get (i), str);
      anim.UpdateNodeColor (wifiStaNodes.Get (i), 255, 0, 0);
    }
    for (uint8_t i = 0; i < wifiApNode.GetN (); ++i)
    {
      anim.UpdateNodeDescription (wifiApNode.Get (i), "AP");
      anim.UpdateNodeColor (wifiApNode.Get (i), 0, 255, 0);
    }
    anim.UpdateNodeDescription (p2pNodes.Get (1), "Server");
    anim.UpdateNodeColor (p2pNodes.Get (1), 0, 0, 255);
    anim.SetMaxPktsPerTraceFile(9999999);
  }

  Simulator::Stop (Seconds (SIMULATION_TIME + 2));

  // Tracing
  if (tracing == true && DEBUG_MODE)
    phy.EnablePcap ("ASG3_TraceFile", wifiApNode);

  // Flow Monitor
  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowMonitorHelper;
  flowMonitor = flowMonitorHelper.InstallAll ();

  NS_LOG_INFO("Starting simulation.");
  Simulator::Run ();
  PrintStats (flowMonitorHelper, true, nFlows, sinkPort, enableRtsCts);
  Simulator::Destroy ();

  return 0;
}

/*
 * / breif: Logs flow monitor stats
 *
 * Function logs the flow monitor stats collected for different parameters
 * in their respective files.
 *
 * */
void
PrintStats (FlowMonitorHelper &flowMonitorHelper, bool perFlowInfo, int nFlows, int sinkPort, unsigned int enableRtsCts)
{
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowMonitorHelper.GetClassifier());
  Ptr<FlowMonitor> monitor = flowMonitorHelper.GetMonitor ();
  std::map < FlowId, FlowMonitor::FlowStats > stats = monitor->GetFlowStats();
  uint64_t droppedULPkts = 0;
  uint64_t droppedDLPkts = 0;
  float sumULThroughput = 0;
  float sumDLThroughput = 0;
  FILE *fpULThr = NULL;
  FILE *fpULCol = NULL;
  FILE *fpULPktD = NULL;
  FILE *fpDLThr = NULL;
  FILE *fpDLCol = NULL;
  FILE *fpDLPktD = NULL;

   if(enableRtsCts) // RTS CTS Enabled
   {
     if((fpULThr = fopen("all_sim_th_ul_rts_cts.txt", "a")) == NULL || (fpULCol = fopen("all_sim_col_ul_rts_cts.txt", "a")) == NULL || (fpULPktD = fopen("all_sim_drp_ul_rts_cts.txt", "a")) == NULL ||
        (fpDLThr = fopen("all_sim_th_dl_rts_cts.txt", "a")) == NULL || (fpDLCol = fopen("all_sim_col_dl_rts_cts.txt", "a")) == NULL || (fpDLPktD = fopen("all_sim_drp_dl_rts_cts.txt", "a")) == NULL)
       {
          std::cout << "Cannot open some file Exiting!" << std::endl;
          exit(0);
       }
   }
   else // RTS CTS Disabled
   {
       if((fpULThr = fopen("all_sim_th_ul_wo_rts_cts.txt", "a")) == NULL || (fpULCol = fopen("all_sim_col_ul_wo_rts_cts.txt", "a")) == NULL || (fpULPktD = fopen("all_sim_drp_ul_wo_rts_cts.txt", "a")) == NULL ||
          (fpDLThr = fopen("all_sim_th_dl_wo_rts_cts.txt", "a")) == NULL || (fpDLCol = fopen("all_sim_col_dl_wo_rts_cts.txt", "a")) == NULL || (fpDLPktD = fopen("all_sim_drp_dl_wo_rts_cts.txt", "a")) == NULL)
       {
           std::cout << "Cannot open some file Exiting!" << std::endl;
           exit(0);
       }
   }

   for(std::map< FlowId, FlowMonitor::FlowStats>::iterator flow = stats.begin(); flow != stats.end(); flow++)
    {
     Ipv4FlowClassifier::FiveTuple  tuple = classifier->FindFlow(flow->first);
	 
     if(abs(tuple.sourcePort - sinkPort) > 30) // Uplink Traffic
       {
          sumULThroughput += ( ((double)flow->second.rxBytes * 8) / ((flow->second.timeLastRxPacket.GetSeconds () - flow->second.timeFirstRxPacket.GetSeconds()) * 1024 * 1024) ); // Throughput UL
          droppedULPkts += flow->second.lostPackets; // Dropped Packets UL
       }
     else // Downlink Traffic
       {
          sumDLThroughput += ( ((double)flow->second.rxBytes * 8) / ((flow->second.timeLastRxPacket.GetSeconds () - flow->second.timeFirstRxPacket.GetSeconds()) * 1024 * 1024) ); // Throughput DL
          droppedDLPkts += flow->second.lostPackets; // Dropped Packets UL
       }
     }

   if(DEBUG_MODE)
   {
       std::cout<< "THROUGHPUT (UL): flows: " << nFlows << " | " << (float)sumULThroughput/nFlows << " Mbps" << std::endl;
       std::cout<< "THROUGHPUT (DL): flows: " << nFlows << " | " << (float)sumDLThroughput/nFlows << " Mbps" << std::endl;
       std::cout<< "DROPPED PACKETS  (UL): flows: " << nFlows << " | " << (float)droppedULPkts << " packets" << std::endl;
       std::cout<< "DROPPED PACKETS  (DL): flows: " << nFlows << " | " << (float)droppedDLPkts << " packets" << std::endl;
       std::cout<< "COLLISION PACKETS (UL): flows: " << nFlows << " | " << (float)g_allStaCollisionPkts << " packets" << std::endl;
       std::cout<< "COLLISION PACKETS (DL): flows: " << nFlows << " | " << (float)0 << " packets" << std::endl;
   }

   /* Report result*/
   fprintf(fpULThr,  "%d %f\n", nFlows, (float)sumULThroughput/nFlows);
   fprintf(fpDLThr,  "%d %f\n", nFlows, (float)sumDLThroughput/nFlows);
   fprintf(fpULPktD, "%d %f\n", nFlows, (float)droppedULPkts);
   fprintf(fpDLPktD, "%d %f\n", nFlows, (float)droppedDLPkts);
   fprintf(fpULCol,  "%d %f\n", nFlows, (float)g_allStaCollisionPkts);
   fprintf(fpDLCol,  "%d %f\n", nFlows, (float)0);
   
   /* Close File pointers*/
   fclose(fpULThr);
   fclose(fpDLThr);
   fclose(fpULPktD);
   fclose(fpDLPktD);
   fclose(fpULCol);
   fclose(fpDLCol);
}


/*
 * / breif: Allocation of random 2D Coordinates
 *
 * Function generates random coordinates along the boundary of a disc with
 * (xCoor, yCoor) as Centre and 'radius' as Radius of the disc.
 *
 * */
void 
SetRandomCircularCoordinates(Ptr<ListPositionAllocator> positionAlloc, double xCoor, double yCoor, double radius)
{
    double theta = 0;
    double angle = (360 / MAX_STA_NODES);

    for(theta=0; theta<360; theta += angle)
        positionAlloc->Add (Vector ((double)(xCoor + radius*cos(theta)), (double)(yCoor + radius*sin(theta)), 0.0));
}

/* 
 * / breif: Generation of Random Numbers 
 * 
 * Function generates random numbers which will be used to activate 
 * wifi-stations.
 *
 * */
void 
GetRandomNumbers(uint64_t *randomNumbers, const uint64_t count)
{
    uint64_t i,j,k;
    uint64_t rangeOf2 = 1;

    while(rangeOf2 < MAX_STA_NODES)
        rangeOf2 *= 2;

    srand ( (unsigned)time ( NULL ) );
    for(i = rand() % MAX_STA_NODES, j = 0, k = 0; k < count;)
    {
        i = (i + j)%rangeOf2;
        j++;
        if( i < MAX_STA_NODES)
            randomNumbers[k++] = i;
    }
}

/* 
 * / breif: Counts all Stations Collision Packets dropped
 *
 * Maintains the number of packets dropped due to collision at 
 * Phy Channel, it only counts the Collision of packets due to
 * interference by other stations, hack was used to acheive this.
 *
 * */
static void
StaPhyRxDrop (Ptr<const Packet> p)
{
  g_allStaCollisionPkts++;
}

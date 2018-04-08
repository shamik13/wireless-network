/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
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
 *
 * Author: Jaume Nin <jaume.nin@cttc.cat>
 */

#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/lte-enb-phy.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/netanim-module.h"
//#include "ns3/visualizer-module.h"//Require for visualization

using namespace ns3;

/**
 * Sample simulation script for LTE + EPC. It instantiates several eNodeB,
 * attaches one UE per eNodeB starts a flow for each UE to  and from a remote host.
 * It also  starts yet another flow between each UE pair.
 */
NS_LOG_COMPONENT_DEFINE ("EpcFirstExample");



void
PrintGnuplottableUeListToFile (std::string filename)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
      Ptr<Node> node = *it;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<LteUeNetDevice> uedev = node->GetDevice (j)->GetObject <LteUeNetDevice> ();
          if (uedev)
            {
              Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
              outFile << "set label \"" << uedev->GetImsi ()
                      << "\" at "<< pos.x << "," << pos.y << " left font \"Helvetica,4\" textcolor rgb \"grey\" front point pt 1 ps 0.3 lc rgb \"grey\" offset 0,0"
                      << std::endl;
            }
        }
    }
}

void
PrintGnuplottableEnbListToFile (std::string filename)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
      Ptr<Node> node = *it;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<LteEnbNetDevice> enbdev = node->GetDevice (j)->GetObject <LteEnbNetDevice> ();
          if (enbdev)
            {
              Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
              outFile << "set label \"" << enbdev->GetCellId ()
                      << "\" at "<< pos.x << "," << pos.y
                      << " left font \"Helvetica,4\" textcolor rgb \"white\" front  point pt 2 ps 0.3 lc rgb \"white\" offset 0,0"
                      << std::endl;
            }
        }
    }
}






void printStats(FlowMonitorHelper &flowmon_helper, bool perFlowInfo)
{
	  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon_helper.GetClassifier());
	  std::string proto;
	  Ptr<FlowMonitor> monitor = flowmon_helper.GetMonitor ();
	  std::map < FlowId, FlowMonitor::FlowStats > stats = monitor->GetFlowStats();
	  double totalTimeReceiving;
	  uint64_t totalPacketsReceived, totalPacketsDropped, totalBytesReceived;

	  totalBytesReceived = 0, totalPacketsDropped = 0, totalPacketsReceived = 0, totalTimeReceiving = 0;
	  for (std::map< FlowId, FlowMonitor::FlowStats>::iterator flow = stats.begin(); flow != stats.end(); flow++)
	  {
	    Ipv4FlowClassifier::FiveTuple  t = classifier->FindFlow(flow->first);
	    switch(t.protocol)
	     {
	     case(6):
	         proto = "TCP";
	         break;
	     case(17):
	         proto = "UDP";
	         break;
	     default:
	         exit(1);
	     }
	     totalBytesReceived += (double) flow->second.rxBytes * 8;
	     totalTimeReceiving += flow->second.timeLastRxPacket.GetSeconds ();
	     totalPacketsReceived += flow->second.rxPackets;
	     totalPacketsDropped += flow->second.txPackets - flow->second.rxPackets;
	     if (perFlowInfo) {
	       std::cout << "FlowID: " << flow->first << " (" << proto << " "
	                 << t.sourceAddress << " / " << t.sourcePort << " --> "
	                 << t.destinationAddress << " / " << t.destinationPort << ")" << std::endl;
	       std::cout << "  Tx Bytes: " << flow->second.txBytes << std::endl;
	       std::cout << "  Rx Bytes: " << flow->second.rxBytes << std::endl;
	       std::cout << "  Tx Packets: " << flow->second.txPackets << std::endl;
	       std::cout << "  Rx Packets: " << flow->second.rxPackets << std::endl;
	       std::cout << "  Time LastRxPacket: " << flow->second.timeLastRxPacket.GetSeconds () << "s" << std::endl;
	       std::cout << "  Lost Packets: " << flow->second.lostPackets << std::endl;
	       std::cout << "  Pkt Lost Ratio: " << ((double)flow->second.txPackets-(double)flow->second.rxPackets)/(double)flow->second.txPackets << std::endl;
	       std::cout << "  Throughput: " << ( ((double)flow->second.rxBytes*8) / (flow->second.timeLastRxPacket.GetSeconds ()) ) << "bps" << std::endl;
	       std::cout << "  Mean{Delay}: " << (flow->second.delaySum.GetSeconds()/flow->second.rxPackets) << std::endl;
	       std::cout << "  Mean{Jitter}: " << (flow->second.jitterSum.GetSeconds()/(flow->second.rxPackets)) << std::endl;
	     }
	   }
}

int main (int argc, char *argv[])
{
  uint16_t numberOfeNodeBs = 4;
  uint16_t numberOfUEs = 4;
  double simTime = 10.0;
  double distance = 1000;
  double interPacketInterval = 100;
  //{ PyViz v; }//Require for visualization
  // Command line arguments
  CommandLine cmd;
  cmd.AddValue("numberOfNodes", "Number of eNodeBs + UE pairs", numberOfeNodeBs);
  cmd.AddValue("simTime", "Total duration of the simulation [s])", simTime);
  cmd.AddValue("distance", "Distance between eNBs [m]", distance);
  cmd.AddValue("interPacketInterval", "Inter packet interval [ms])", interPacketInterval);
  cmd.Parse(argc, argv);

  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create(numberOfeNodeBs);
  ueNodes.Create(numberOfUEs);

    //Placing eNB
  Ptr<ListPositionAllocator> enbpositionAlloc = CreateObject<ListPositionAllocator> ();
  enbpositionAlloc->Add (Vector(280, 280, 0));
  enbpositionAlloc->Add(Vector( 320, 280,0));
  enbpositionAlloc->Add(Vector(280,320,0));
  enbpositionAlloc->Add(Vector(320,320,0));

  
  //Placing UEs
  Ptr<ListPositionAllocator> uepositionAlloc = CreateObject<ListPositionAllocator> ();
  uepositionAlloc->Add (Vector(290, 290, 0));
  uepositionAlloc->Add (Vector(310, 290, 0));
  uepositionAlloc->Add (Vector(290, 310, 0));
  uepositionAlloc->Add (Vector(310, 310, 0));
 
  //Install Mobility Model
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(enbpositionAlloc);
  mobility.Install(enbNodes);
  mobility.SetPositionAllocator(uepositionAlloc);
  mobility.Install(ueNodes);

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");
 // lteHelper->SetSchedulerType ("ns3::PfFfMacScheduler");
//  lteHelper->SetSchedulerType ("ns3::FdMtFfMacScheduler"); //FdMtFfMacScheduler

  
  lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (25));
  Ptr<Node> pgw = epcHelper->GetPgwNode ();
  Ptr<ListPositionAllocator> pgwpositionAlloc = CreateObject<ListPositionAllocator> ();
  pgwpositionAlloc->Add(Vector(600,600,0));
  mobility.SetPositionAllocator(pgwpositionAlloc);
  mobility.Install(pgw);

   // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  Ptr<ListPositionAllocator> rHpositionAlloc = CreateObject<ListPositionAllocator> ();
  rHpositionAlloc->Add(Vector(800,800,0));
  mobility.SetPositionAllocator(rHpositionAlloc);
  mobility.Install(remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);

  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  //Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1); //uncomment the below line for uplink of client

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);
  for(uint16_t i = 0; i < numberOfeNodeBs; i++){
    Ptr<LteEnbPhy> enb1Phy = enbLteDevs.Get (i)->GetObject<LteEnbNetDevice>()->GetPhy ();
    enb1Phy->SetTxPower (30.0); 
   }
  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // Attach all UEs to eNodeB
  for (uint16_t i = 0; i < numberOfUEs; i++)
      {
        lteHelper->Attach (ueLteDevs.Get(i), enbLteDevs.Get(i));
        // side effect: the default EPS bearer will be activated
      }

    lteHelper->AddX2Interface (enbNodes);
    UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (remoteHostContainer);//Install echoserver on remote host
  serverApps.Start (Seconds (0.1));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (internetIpIfaces.GetAddress(1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1000));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.001)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1500));


  for(int i_count=0;i_count<4;i_count++)//Installing echo client on all UE's
  {
	ApplicationContainer clientApps = echoClient.Install (ueNodes.Get(i_count));
  clientApps.Start (Seconds (0.1));
  clientApps.Stop (Seconds (10.0));
  
  }
  
    
  
  lteHelper->EnableTraces ();

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> Monitor;
  Monitor = flowmon.Install(ueNodes);
  Monitor = flowmon.Install(remoteHostContainer);

  Ptr<RadioEnvironmentMapHelper> remHelper;
  if (false)
    {

      PrintGnuplottableEnbListToFile ("enbs.txt");
      PrintGnuplottableUeListToFile ("ues.txt");


      remHelper = CreateObject<RadioEnvironmentMapHelper> ();
      remHelper->SetAttribute ("ChannelPath", StringValue ("/ChannelList/1"));
      remHelper->SetAttribute ("OutputFile", StringValue ("myrem.rem"));

      remHelper->SetAttribute ("XMin", DoubleValue (-150));
      remHelper->SetAttribute ("XMax", DoubleValue (750));
 //     remHelper->SetAttribute ("XRes", UintegerValue (5));
      remHelper->SetAttribute ("YMin", DoubleValue (-150));
      remHelper->SetAttribute ("YMax", DoubleValue (750));
   //   remHelper->SetAttribute ("YRes", UintegerValue (5));
      remHelper->SetAttribute ("Z", DoubleValue (1.5));
      remHelper->Install ();
      // simulation will stop right after the REM has been generated
    }


  else
    {
      Simulator::Stop (Seconds (simTime));
    }

  Simulator::Run();
  printStats(flowmon,true);
  Simulator::Destroy();
  return 0;

}


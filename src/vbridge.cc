//
// i take in as parameters a list of taps (each associated with an lxc)
// i log basic packet info to stdout (src, dst, and recipient-node for now)
// fsim will make a temporary copy of me in the ns3 scratch folder at runtime
//
#include <iostream>
#include <stdio.h>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/tap-bridge-module.h"
using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("vmtest");
int main (int argc, char *argv[])
{
  //CommandLine cmd;
  //cmd.Parse (argc, argv);
  LogComponentEnable("vmtest",LOG_LEVEL_ALL);
  NS_LOG_INFO ("boop");
  std::cout << "simulating a network connection for 1 hr" << "\n";
  std::cout << "between these taps:\n\n";
  for(int i=1; i<argc; i++){ std::cout << "  " << argv[i] << "\n"; }
  //
  // We are interacting with the outside, real, world.  This means we have to
  // interact in real-time and therefore means we have to use the real-time
  // simulator and take the time to calculate checksums.
  //
  GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));
  //CUSTOM - this gives us a tiny bit of packet info (src, dst, node-who-received-it)
  LogComponentEnable ("TapBridge", LOG_LEVEL_ALL);
  //END CUSTOM
  //
  // Create two ghost nodes.  The first will represent the virtual machine host
  // on the left side of the network; and the second will represent the VM on
  // the right side.
  //
  // ACTUALLY NOW IT'S n GHOST NODES, WHERE n=argc-1
  //
  NodeContainer nodes;
  nodes.Create (argc-1);
  //
  // Use a CsmaHelper to get a CSMA channel created, and the needed net
  // devices installed on both of the nodes.  The data rate and delay for the
  // channel can be set through the command-line parser.  For example,
  //
  // ./waf --run "tap=csma-virtual-machine --ns3::CsmaChannel::DataRate=10000000"
  //
  CsmaHelper csma;
  //CUSTOM - we can set these up however we want
  //currently, this simulated network connections are all ethernet cables connected to a single router. 
  //not very realistic. someday this will be BGP->ANs->POPs->LANs (i think)
  //
  csma.SetChannelAttribute ("DataRate", DataRateValue (10*1024*1024));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (300)));
  //END CUSTOM
  NetDeviceContainer devices = csma.Install (nodes);
  //
  // Use the TapBridgeHelper to connect to the pre-configured tap devices for
  // the left side.  We go with "UseBridge" mode since the CSMA devices support
  // promiscuous mode and can therefore make it appear that the bridge is
  // extended into ns-3.  The install method essentially bridges the specified
  // tap to the specified CSMA device.
  //
  TapBridgeHelper tapBridge;
  tapBridge.SetAttribute ("Mode", StringValue ("UseBridge"));
  tapBridge.SetAttribute ("DeviceName", StringValue (argv[1]));
  tapBridge.Install (nodes.Get (0), devices.Get (0));
  //
  // Connect the right side tap to the right side CSMA device on the right-side
  // ghost node.
  //
  //CUSTOM
  //actually now we connect them all CSMA_device[i] -> ghost_node[i]
  //
  for(int i=1; i<argc-1; i++){
    tapBridge.SetAttribute ("DeviceName", StringValue (argv[i+1]));
    tapBridge.Install (nodes.Get (i), devices.Get (i));
  }
  //END CUSTOM
  //
  // Run the simulation for one hour to give the user time to play around
  // this could be eternal, but for now it's an hour, cuz i dunno why (it was 10min).
  //
  Simulator::Stop (Seconds (3600));
  Simulator::Run ();
  Simulator::Destroy ();
}

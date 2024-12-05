#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/point-to-point-module.h"

#include "dijkstra.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

using namespace ns3;

/**
    Ahmet Emre Eser - EE414 hmw4
                                 **/

NS_LOG_COMPONENT_DEFINE("hw4");

int
main(int argc, char* argv[])
{
    LogComponentEnable ("hw4", LOG_LEVEL_INFO);

    Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(210));
    Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("448kb/s"));

    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    NS_LOG_INFO("DIJKSTRA BEGIN");

    // weights 
    const int wAB = 2, wAC = 6, wAD = 2, wBD = 3, wBE = 5, wCD = 4, wDE = 7;

    Dijkstra::Graph network(5);
    Dijkstra dijkstra(network);

    dijkstra.addEdge('A', 'B', wAB); // A -> B
    dijkstra.addEdge('A', 'C', wAC); // A -> C
    dijkstra.addEdge('A', 'D', wAD); // A -> D
    dijkstra.addEdge('B', 'D', wBD); // B -> D
    dijkstra.addEdge('B', 'E', wBE); // B -> E
    dijkstra.addEdge('C', 'D', wCD); // C -> D
    dijkstra.addEdge('D', 'E', wDE); // D -> E

    Dijkstra::DistanceVector distances = dijkstra(0);

    std::cout << "Dijkstra results:\n";
    for (int i = 0; i < (int) (distances.size()); i++) {
        const char lookup[] = {'A', 'B', 'C', 'D', 'E'};
        std::cout << "Distance from A to " << lookup[i] << ": " << distances[i] << std::endl;
    }

    NS_LOG_INFO("DIJKSTRA END");
    NS_LOG_INFO("NETWORK SIM CONFIG BEGIN");

    NS_LOG_INFO("Create nodes.");
    NodeContainer c;
    c.Create(5);

    NodeContainer ncAB = NodeContainer(c.Get(0), c.Get(1));
    NodeContainer ncAC = NodeContainer(c.Get(0), c.Get(2));
    NodeContainer ncAD = NodeContainer(c.Get(0), c.Get(3));
    NodeContainer ncBD = NodeContainer(c.Get(1), c.Get(3));
    NodeContainer ncBE = NodeContainer(c.Get(1), c.Get(4));
    NodeContainer ncCD = NodeContainer(c.Get(2), c.Get(3));
    NodeContainer ncDE = NodeContainer(c.Get(3), c.Get(4));

    // Enable OLSR
    NS_LOG_INFO("Enabling OLSR Routing.");
    OlsrHelper olsr;

    Ipv4StaticRoutingHelper staticRouting;

    Ipv4ListRoutingHelper list;
    list.Add(staticRouting, 0);
    list.Add(olsr, 10);

    InternetStackHelper internet;
    internet.SetRoutingHelper(list); // has effect on the next Install ()
    internet.Install(c);

    // We create the channels first without any IP addressing information
    NS_LOG_INFO("Create channels.");
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("210kbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer ndAB = p2p.Install(ncAB); // A -> B (dev 1 for A, dev 1 for B)
    p2p.SetDeviceAttribute("DataRate", StringValue("70kbps"));
    NetDeviceContainer ndAC = p2p.Install(ncAC);
    p2p.SetDeviceAttribute("DataRate", StringValue("210kbps"));
    NetDeviceContainer ndAD = p2p.Install(ncAD);
    p2p.SetDeviceAttribute("DataRate", StringValue("140kbps"));
    NetDeviceContainer ndBD = p2p.Install(ncBD);
    p2p.SetDeviceAttribute("DataRate", StringValue("84kbps"));
    NetDeviceContainer ndBE = p2p.Install(ncBE); // B -> E (dev 3 for B, dev 1 for E)
    p2p.SetDeviceAttribute("DataRate", StringValue("105kbps"));
    NetDeviceContainer ndCD = p2p.Install(ncCD);
    p2p.SetDeviceAttribute("DataRate", StringValue("60kbps"));
    NetDeviceContainer ndDE = p2p.Install(ncDE);

    // Later, we add IP addresses.
    NS_LOG_INFO("Assign IP Addresses.");
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer iAB = ipv4.Assign(ndAB);

    ipv4.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer iAC = ipv4.Assign(ndAC);

    ipv4.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer iAD = ipv4.Assign(ndAD);

    ipv4.SetBase("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer iBD = ipv4.Assign(ndBD);

    ipv4.SetBase("10.1.5.0", "255.255.255.0");
    Ipv4InterfaceContainer iBE = ipv4.Assign(ndBE);

    ipv4.SetBase("10.1.6.0", "255.255.255.0");
    Ipv4InterfaceContainer iCD = ipv4.Assign(ndCD);

    ipv4.SetBase("10.1.7.0", "255.255.255.0");
    Ipv4InterfaceContainer iDE = ipv4.Assign(ndDE);

    // Create the OnOff application to send UDP datagrams of size
    // 210 bytes at a rate of 448 Kb/s from n0 to n4
    NS_LOG_INFO("Create Applications.");
    uint16_t port = 9; // Discard port (RFC 863)

    OnOffHelper onoff1("ns3::UdpSocketFactory", InetSocketAddress(iBE.GetAddress(1), port));
    onoff1.SetConstantRate(DataRate("448kb/s"));

    ApplicationContainer onOffApp1 = onoff1.Install(c.Get(0)); // A
    onOffApp1.Start(Seconds(0.1));
    onOffApp1.Stop(Seconds(5.0));

    // Create a similar flow from n3 to n1, starting at time 1.1 seconds
    // OnOffHelper onoff2("ns3::UdpSocketFactory", InetSocketAddress(iAB.GetAddress(0), port));
    // onoff2.SetConstantRate(DataRate("448kb/s"));

    // ApplicationContainer onOffApp2 = onoff2.Install(c.Get(4)); // E
    // onOffApp2.Start(Seconds(0.1));
    // onOffApp2.Stop(Seconds(5.0));

    // Create packet sinks to receive these packets
    PacketSinkHelper sink("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApp = sink.Install(c.Get(4)); // E
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(5.1));
    // NodeContainer sinks = NodeContainer(c.Get(4), c.Get(1));
    // ApplicationContainer sinkApps = sink.Install(sinks);
    // sinkApps.Start(Seconds(0.0));
    // sinkApps.Stop(Seconds(5.1));

    AsciiTraceHelper ascii;
    p2p.EnableAsciiAll(ascii.CreateFileStream("hw4.tr"));
    p2p.EnablePcapAll("hw4");

    Simulator::Stop(Seconds(5.2));

    NS_LOG_INFO("Run Simulation.");
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");

    return 0;
}

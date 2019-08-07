#include "simpleMeshTest.h"

#define	PROG_DIR "wlan/exp06/"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MeshDot11sSim");

MeshDot11sSim::MeshDot11sSim () :
	m_xSize       (3),
	m_ySize       (3),
	m_step        (170),
	m_randomStart (0.1),
	m_totalTime   (60.1),
	m_packetSize  (512),
	m_nIfaces     (1),
	m_chan        (false),
	m_txrate      ("512kbps"),
	m_root        ("ff:ff:ff:ff:ff:ff"),
	m_showRtable  (false)
{
	NS_LOG_FUNCTION(this);
}

void
MeshDot11sSim::RouteDiscoveryTimeSniffer (std::string context, Time time)
{
	NS_LOG_FUNCTION(this);

	std::cout<<"RouteDiscoveryTime  :"<<time.GetSeconds()<<std::endl;
}
void
MeshDot11sSim::Configure (int argc, char *argv[])
{
	NS_LOG_FUNCTION(this);

	CommandLine cmd;
	cmd.AddValue ("xSize"     , "number of nodes in each row" , m_xSize);
	cmd.AddValue ("ySize"     , "number of nodes in each column" , m_ySize);
	cmd.AddValue ("distance"  , "distance between two nodes"  , m_step);
	cmd.AddValue ("rate"      , "maximum trasmission rate", m_txrate);
	cmd.AddValue ("root"      , "Mac address of root mesh point in HWMP", m_root);
	cmd.AddValue ("channels"  , "Use different frequency channels for different interfaces.[0]", m_chan);
	cmd.AddValue ("interfaces", "Number of radio interfaces used by each mesh point.[1]", m_nIfaces);
	cmd.AddValue ("showRtable", "Show HWMP Routing tables.[0]", m_showRtable);
	cmd.Parse (argc, argv);
}

void
MeshDot11sSim::CreateTopologyNodes ()
{
	NS_LOG_FUNCTION(this);

	// Create mesh nodes
	n = new Ptr<Node> [m_xSize*m_ySize];
        for (uint32_t i = 0; i < m_xSize*m_ySize; ++i) {
                n[i] = CreateObject<Node> ();
                n[i]->AggregateObject (CreateObject<ConstantPositionMobilityModel> ());
                meshNodes.Add (n[i]);
        }

	MobilityHelper mobility;
	mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                "MinX"      , DoubleValue (0.0),
                "MinY"      , DoubleValue (100.0),
                "DeltaX"    , DoubleValue (m_step),
                "DeltaY"    , DoubleValue (m_step),
                "GridWidth" , UintegerValue (m_xSize),
                "LayoutType", StringValue ("RowFirst"));

	//mobility.SetPositionAllocator(&myListPositionAllocator);
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (meshNodes);

	// Create BGR nodes
        bgr = CreateObject<Node> ();

        //move model
        ListPositionAllocator pos;
        Vector3D wiredPoint (0.0, 0.0, 0.0);
        pos.Add(wiredPoint);
        mobility.SetPositionAllocator(&pos);
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        mobility.Install(bgr);
}

void
MeshDot11sSim::ConfigureMeshLayer ()
{
	NS_LOG_FUNCTION(this);

	double   m_txpower = 18.0; // dbm

//	LogComponentEnable("HwmpRtable", LogLevel(LOG_LEVEL_DEBUG|
//		LOG_PREFIX_NODE|LOG_PREFIX_TIME|LOG_PREFIX_FUNC));
//	LogComponentEnable("HwmpProtocol", LogLevel(LOG_LEVEL_DEBUG|
//		LOG_PREFIX_NODE|LOG_PREFIX_TIME|LOG_PREFIX_FUNC));
//	LogComponentEnable("PeerManagementProtocol", LogLevel(LOG_LEVEL_DEBUG|
//		LOG_PREFIX_NODE|LOG_PREFIX_TIME|LOG_PREFIX_FUNC));
//	LogComponentEnable("MeshWifiInterfaceMac", LogLevel(LOG_LEVEL_DEBUG|
//		LOG_PREFIX_NODE|LOG_PREFIX_TIME|LOG_PREFIX_FUNC));

	// Configure YansWifiChannel
	YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
	phy.Set ("EnergyDetectionThreshold", DoubleValue (-89.0) );
	phy.Set ("CcaMode1Threshold"       , DoubleValue (-62.0) );
	phy.Set ("TxGain"                  , DoubleValue (1.0) );
	phy.Set ("RxGain"                  , DoubleValue (1.0) );
	phy.Set ("TxPowerLevels"           , UintegerValue (1) );
	phy.Set ("TxPowerEnd"              , DoubleValue (m_txpower) );
	phy.Set ("TxPowerStart"            , DoubleValue (m_txpower) );
	phy.Set ("RxNoiseFigure"           , DoubleValue (7.0) );
	YansWifiChannelHelper channel;
	channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
	channel.AddPropagationLoss  ("ns3::LogDistancePropagationLossModel",
		"Exponent", StringValue ("2.7"));
	phy.SetChannel (channel.Create ());

	// Configure the parameters of the Peer Link
	Config::SetDefault ("ns3::dot11s::PeerLink::MaxBeaconLoss"   , UintegerValue (20));
	Config::SetDefault ("ns3::dot11s::PeerLink::MaxRetries"      , UintegerValue (4));
	Config::SetDefault ("ns3::dot11s::PeerLink::MaxPacketFailure", UintegerValue (5));

	// Configure the parameters of the Peer Management Protocol
	Config::SetDefault ("ns3::dot11s::PeerManagementProtocol::EnableBeaconCollisionAvoidance",
		BooleanValue (false));

	// Configure the parameters of the HWMP -------------------------------------------------
	// set Max Queue Length
	Config::SetDefault ("ns3::dot11s::HwmpProtocol::MaxQueueSize",
		UintegerValue (256));
	// set Lifetime of reactive routing information
	Config::SetDefault ("ns3::dot11s::HwmpProtocol::Dot11MeshHWMPactivePathTimeout",
		TimeValue (Seconds (100)));
	// set Lifetime of poractive routing information
	Config::SetDefault ("ns3::dot11s::HwmpProtocol::Dot11MeshHWMPactiveRootTimeout",
		TimeValue (Seconds (100)));
	// set Maximum number of retries before we suppose the destination to be unreachable
        Config::SetDefault ("ns3::dot11s::HwmpProtocol::Dot11MeshHWMPmaxPREQretries",
                UintegerValue (5));
	// set Maximum number of PREQ receivers, when we send a PREQ as a chain of unicasts
	Config::SetDefault ("ns3::dot11s::HwmpProtocol::UnicastPreqThreshold",
		UintegerValue (10));
	// set Maximum number of broadcast receivers, when we send a broadcast as a chain of unicasts
	Config::SetDefault ("ns3::dot11s::HwmpProtocol::UnicastDataThreshold",
		UintegerValue (5));
	// set Destination only HWMP flag
	Config::SetDefault ("ns3::dot11s::HwmpProtocol::DoFlag",
		BooleanValue (true));
	// set Reply and forward flag
	Config::SetDefault ("ns3::dot11s::HwmpProtocol::RfFlag",
		BooleanValue (false));

	// Stack installer creates all protocols and install them to mesh point device
	mesh = MeshHelper::Default ();
	mesh.SetStandard (WIFI_PHY_STANDARD_80211a);
	if (!Mac48Address (m_root.c_str ()).IsBroadcast ()) {
                mesh.SetStackInstaller ("ns3::Dot11sStack", "Root",
                	Mac48AddressValue (Mac48Address (m_root.c_str ())));
        } else
		mesh.SetStackInstaller ("ns3::Dot11sStack");

	mesh.SetMacType ("RandomStart", TimeValue (Seconds(m_randomStart)));
	mesh.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
		"DataMode", StringValue ("OfdmRate6Mbps"),
		"RtsCtsThreshold", UintegerValue (2500));

	// Set number of interfaces - default is single-interface mesh point
	mesh.SetNumberOfInterfaces (m_nIfaces);

	//If multiple channels is activated
	if (m_chan)
		mesh.SetSpreadInterfaceChannels (MeshHelper::SPREAD_CHANNELS);
	else
		mesh.SetSpreadInterfaceChannels (MeshHelper::ZERO_CHANNEL);

	// Install protocols and return container if MeshPointDevices
	meshDevices = mesh.Install (phy, meshNodes);

	//connect the mesh to the Internet---------------------------------------
	PointToPointHelper p2p;
        p2p.SetDeviceAttribute  ("DataRate", StringValue ("5Mbps"));
        p2p.SetChannelAttribute ("Delay"   , StringValue ("2ms"));

        // set portal node(n[0])
        p2pDevices = p2p.Install (bgr, n[0]);
}

void
MeshDot11sSim::InstallInternetStack ()
{
	NS_LOG_FUNCTION(this);

	//Install the internet protocol stack on all nodes
	InternetStackHelper internetStack;
	internetStack.InstallAll();

	//Assign IP addresses to the devices interfaces
	Ipv4AddressHelper address;
	address.SetBase ("192.168.1.0", "255.255.255.0");
	address.Assign (p2pDevices);

	address.SetBase ("10.10.1.0", "255.255.255.0");
	address.Assign (meshDevices);

	// setup a static route from EGR to mesh network(10.10.1.0/24) via portal node(n[0]).
	Ipv4StaticRoutingHelper staticRouting;
	Ptr<Ipv4StaticRouting> bgrStatic = staticRouting.GetStaticRouting (bgr->GetObject<Ipv4> ());
	bgrStatic->AddNetworkRouteTo (Ipv4Address ("10.10.1.0"),
		Ipv4Mask ("255.255.255.0"), Ipv4Address ("192.168.1.2"), 1);

	// set default route to exterior networks for each mesh node
	// 10.10.1.1(n[0]) is a portal node to the exteriar gateway router of internet
	Ipv4Address gateway ("10.10.1.1");
	Ptr<Ipv4StaticRouting> meshStatic;
        for (uint32_t i = 1; i < m_xSize*m_ySize; ++i) {
		meshStatic = staticRouting.GetStaticRouting (n[i]->GetObject<Ipv4> ());
		meshStatic->SetDefaultRoute(gateway, 1);
	}
}

void
MeshDot11sSim::SetUpTcpApplication(Ptr <Node> s, Ptr <Node> m, std::string proto)
{
	NS_LOG_FUNCTION(this);

	// get node n's IP address as source address
	Ipv4InterfaceAddress sadr = s->GetObject <Ipv4> ()->GetAddress(1, 0);
	Address srcAddr (InetSocketAddress (sadr.GetLocal(), 8888));

	// get node m's IP address as destination address
	Ipv4InterfaceAddress madr = m->GetObject <Ipv4> ()->GetAddress(1, 0);
	Address remoteAddr (InetSocketAddress (madr.GetLocal(), 8888));

	NS_LOG_UNCOND("Set " << proto << " flow: " << sadr.GetLocal() << " --> " << madr.GetLocal());

	OnOffHelper ftp = OnOffHelper ("ns3::TcpSocketFactory", Address());
	if(proto == "UDP")
		ftp = OnOffHelper ("ns3::UdpSocketFactory", Address());
	else if (proto != "TCP") {
		NS_LOG_UNCOND("ERROR: no such protocol!");
		exit(1);
	}

	ftp.SetAttribute ("OnTime",
	StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
	ftp.SetAttribute ("OffTime",
	StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
	ftp.SetAttribute ("Remote", AddressValue(remoteAddr));
	ApplicationContainer src = ftp.Install (s);
	src.Start (Seconds (30.1));
	src.Stop  (Seconds (60.1));

	// Create an optional packet sink to receive these packets
	PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory",
	Address (InetSocketAddress (Ipv4Address::GetAny (), 8888)));
	ApplicationContainer sink = sinkHelper.Install (m);
	sink.Start (Seconds (30.1));
	sink.Stop  (Seconds (60.1 ));
}

void
MeshDot11sSim::RunSim (int argc, char *argv[])
{
	NS_LOG_FUNCTION(this);

	Configure (argc, argv);

	// use ConfigStore
	// following default configuration are same to
	// ./waf --run "exp06-simpleMesh --ns3::ConfigStore::Mode=Save
	//	 --ns3::ConfigStore::Filename=config.txt"
	std::string cf = std::string(PROG_DIR) + "config.txt";
	Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (cf));
	Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
	Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
	ConfigStore config;
  	config.ConfigureDefaults ();

	CreateTopologyNodes ();
	ConfigureMeshLayer();
	InstallInternetStack ();

	//SetUpUdpApplication();
        for (uint16_t i = 1; i < m_xSize*m_ySize; ++i)
		SetUpTcpApplication(n[i], bgr, "TCP");

	// Install FlowMonitor on all nodes
	monitor = flowmon.InstallAll();

	if(m_showRtable)
		Simulator::Schedule (Seconds(m_totalTime),
			&MeshDot11sSim::showHwmpRoutingTables, this);
	Simulator::Schedule (Seconds(m_totalTime), &MeshDot11sSim::Report, this);

	std::string xf = std::string(PROG_DIR) + "simple-hwmp-mesh.xml";
	AnimationInterface anim (xf);

	config.ConfigureAttributes ();

	Config::Connect ("/NodeList/*/DeviceList/0/$ns3::dot11s::HwmpProtocol/RouteDiscoveryTime",
		MakeCallback(&MeshDot11sSim::RouteDiscoveryTimeSniffer, this));

	m_timeStart = clock();

	Simulator::Stop (Seconds (m_totalTime));
	Simulator::Run ();
	FlowMonitoring();
}

void
MeshDot11sSim::FlowMonitoring()
{
	NS_LOG_FUNCTION(this);

	// Define variables to calculate the metrics
	uint32_t totaltxPackets = 0;
	uint32_t totalrxPackets = 0;
	double   totaltxbytes   = 0;
	double   totalrxbytes   = 0;
	double   totaldelay     = 0;
	double   totalrxbitrate = 0;
	double   difftx, diffrx;
	double   pdf_value, rxbitrate_value, delay_value, txbitrate_value;
	double   pdf_total, rxbitrate_total, delay_total;

	std::cout << "---------------------------------------------\n";
	//Print per flow statistics
	monitor->CheckForLostPackets ();
	Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
	std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
	uint32_t k = 0;
	for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin ();
		i != stats.end (); ++i) {
		Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);

		difftx = i->second.timeLastTxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds();
		diffrx = i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstRxPacket.GetSeconds();
		pdf_value = (double) i->second.rxPackets / (double) i->second.txPackets * 100;
		txbitrate_value = (double) i->second.txBytes * 8 / 1024 / difftx;
		if (i->second.rxPackets != 0) {
			rxbitrate_value = (double)i->second.rxPackets * m_packetSize * 8 / 1024 / diffrx;
			delay_value = (double) i->second.delaySum.GetSeconds() / (double) i->second.rxPackets;
		} else {
			rxbitrate_value = 0;
			delay_value = 0;
		}

		// We are only interested in the metrics of the data flows
		if ((!t.destinationAddress.IsSubnetDirectedBroadcast("255.255.255.0"))) {
			k++;

			// Plot the statistics for each data flow
			std::cout << "Flow " << k << " (" << t.sourceAddress << " -> "
				<< t.destinationAddress << ")\n";
			std::cout << "PDF: " << pdf_value << " %\n";
			std::cout << "Average delay: " << delay_value << "s\n";
			std::cout << "Rx bitrate: " << rxbitrate_value << " kbps\n";
			std::cout << "Tx bitrate: " << txbitrate_value << " kbps\n";
			std::cout << "---------------------------------------------\n";

			// Acumulate for average statistics
			totaltxPackets += i->second.txPackets;
			totaltxbytes += i->second.txBytes;
			totalrxPackets += i->second.rxPackets;
			totaldelay += i->second.delaySum.GetSeconds();
			totalrxbitrate += rxbitrate_value;
			totalrxbytes += i->second.rxBytes;
		}
	}

	// Average all nodes statistics
	if (totaltxPackets != 0)
		pdf_total = (double) totalrxPackets / (double) totaltxPackets * 100;
	else
		pdf_total = 0;

	if (totalrxPackets != 0) {
		rxbitrate_total = totalrxbitrate;
		delay_total = (double) totaldelay / (double) totalrxPackets;
	} else {
		rxbitrate_total = 0;
		delay_total = 0;
	}

	//print all nodes statistics
	NS_LOG_UNCOND ("Total PDF: " << pdf_total << " %");
	NS_LOG_UNCOND ("Total Rx bitrate: " << rxbitrate_total << " kbps");
	NS_LOG_UNCOND ("Total Delay: " << delay_total << " s");

	//print all nodes statistics in files
	std::ostringstream os;
	os << PROG_DIR << "data/simple-hwmp_pdf.txt";
	std::ofstream of (os.str().c_str(), std::ios::out | std::ios::app);
	of << pdf_total << "\n";
	of.close ();

	std::ostringstream os2;
	os2 << PROG_DIR << "data/simple-hwmp_delay.txt";
	std::ofstream of2 (os2.str().c_str(), std::ios::out | std::ios::app);
	of2 << delay_total << "\n";
	of2.close ();

	std::ostringstream os3;
	os3 << PROG_DIR << "data/simple-hwmp_throughput.txt";
	std::ofstream of3 (os3.str().c_str(), std::ios::out | std::ios::app);
	of3 << rxbitrate_total << "\n";
	of3.close ();

	Simulator::Destroy ();

	m_timeEnd   = clock();
	m_timeTotal = (m_timeEnd - m_timeStart)/(double) CLOCKS_PER_SEC;

	NS_LOG_UNCOND ("\nSimulation time: " << m_timeTotal << "s");
}

void MeshDot11sSim::Report ()
{
	NS_LOG_FUNCTION(this);

	// Using this function we print detailed statistics of each mesh point device
	// These statistics are used later with an AWK files to extract routing metrics
	for (NetDeviceContainer::Iterator i = meshDevices.Begin (); i != meshDevices.End (); ++i) {
		std::ostringstream os;
		os << PROG_DIR << "data/simple-hwmp-report.xml";
		std::ofstream of;
		of.open (os.str().c_str(), std::ios::out | std::ios::app);
		if (! of.is_open ()) {
			std::cerr << "Error: Can't open file " << os.str() << "\n";
			return;
		}
		mesh.Report(*i, of);

		of.close ();
	}
}

void
MeshDot11sSim::showHwmpRoutingTables()
{
	NS_LOG_FUNCTION(this);

        for (uint32_t i = 0; i < m_xSize*m_ySize; ++i) {
		Ptr<NetDevice> ndev = n[i]->GetDevice(0);
		NS_ASSERT (ndev != 0);
		Ptr<MeshPointDevice> mdev = ndev->GetObject<MeshPointDevice>();
		NS_ASSERT (mdev != 0);
		Ptr<ns3::dot11s::HwmpProtocol> hwmp = mdev->GetObject<ns3::dot11s::HwmpProtocol> ();
		NS_ASSERT (hwmp != 0);
		//hwmp->Report(std::cout);

		hwmp->ReportRtables(std::cout, m_xSize*m_ySize);
	}
}

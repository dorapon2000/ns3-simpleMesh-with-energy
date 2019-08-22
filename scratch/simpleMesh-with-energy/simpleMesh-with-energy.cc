#include "ns3/energy-module.h"
#include "simpleMeshTest.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <iomanip>

#define	PROG_DIR "datatata/"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MeshDot11sSim2");

class MeshDot11sSim2 : public MeshDot11sSim
{
    public:
	// Init test
	MeshDot11sSim2 ();

	// Run test
	void RunSim (int argc, char **argv);
	void Configure (int argc, char ** argv);

    private:
	EnergySourceContainer AttachEnergyModelToDevices(double, double, double);
	void GetRemainingEnergy (double, double);
	void TraceRemainingEnergy(EnergySourceContainer);
	void NotifyDrained();
	double m_remainingEnergy;
	bool   m_energyDrained;
	uint32_t  m_traceNum;
};

MeshDot11sSim2::MeshDot11sSim2 ()
{
	NS_LOG_FUNCTION(this);

	m_energyDrained = false;
	m_traceNum = 0;
}

void
MeshDot11sSim2::Configure (int argc, char *argv[])
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
	cmd.AddValue ("traceNode" , "trace Node's number", m_traceNum);
	cmd.Parse (argc, argv);
}

EnergySourceContainer
MeshDot11sSim2::AttachEnergyModelToDevices(double initialEnergy, double txcurrent, double rxcurrent)
{
	// configure energy source
	BasicEnergySourceHelper basicSourceHelper;
	basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (initialEnergy));
	EnergySourceContainer e = basicSourceHelper.Install (meshNodes);

	// transmit at 0dBm = 17.4mA, receive mode = 19.7mA
	MeshRadioEnergyModelHelper radioEnergyHelper;
	radioEnergyHelper.Set ("TxCurrentA", DoubleValue (txcurrent));
	radioEnergyHelper.Set ("RxCurrentA", DoubleValue (rxcurrent));
	radioEnergyHelper.SetDepletionCallback(
		MakeCallback(&MeshDot11sSim2::NotifyDrained, this));
	DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install (meshDevices, e);
	return e;
}

void // Trace function for remaining energy at node.
MeshDot11sSim2::GetRemainingEnergy (double oldValue, double remainingEnergy)
{
	if(m_energyDrained==false) {
		// show current remaining energy(J)
		std::cout << std::setw(10) << Simulator::Now ().GetSeconds ()
	    		  << "\t " << remainingEnergy <<std::endl;
		m_remainingEnergy = remainingEnergy;
	}
}

void
MeshDot11sSim2::TraceRemainingEnergy(EnergySourceContainer e)
{
	// all energy sources are connected to resource node n>0
	Ptr<BasicEnergySource> basicSourcePtr = DynamicCast<BasicEnergySource> (e.Get (m_traceNum));
	basicSourcePtr->TraceConnectWithoutContext ("RemainingEnergy",
		MakeCallback (&MeshDot11sSim2::GetRemainingEnergy, this));
}

void
MeshDot11sSim2::NotifyDrained()
{
	std::cout <<"Energy was Drained. Stop send.\n";
	m_energyDrained = true;
}

void
MeshDot11sSim2::RunSim (int argc, char *argv[])
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
	// EnergySourceContainer e = AttachEnergyModelToDevices(1.5, 0.0174, 0.0197);
	EnergySourceContainer e = AttachEnergyModelToDevices(50, 0.0174, 0.0197);
	InstallInternetStack ();

	for (uint16_t i = 1; i < m_xSize*m_ySize; ++i)
		SetUpTcpApplication(n[i], bgr, "TCP");

	// Create an optional packet sink to receive these packets
	PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory",
		Address (InetSocketAddress (Ipv4Address::GetAny (), 8888)));
	ApplicationContainer sink = sinkHelper.Install (bgr);
	sink.Start (Seconds (30.1));
	sink.Stop  (Seconds (60.1 ));

	// Install FlowMonitor on all nodes
	monitor = flowmon.InstallAll();

	if(m_showRtable)
		Simulator::Schedule (Seconds(m_totalTime),
			&MeshDot11sSim::showHwmpRoutingTables, this);
	Simulator::Schedule (Seconds(m_totalTime), &MeshDot11sSim::Report, this);

	TraceRemainingEnergy(e);

	std::string xf = std::string(PROG_DIR) + "simple-mesh.xml";
	AnimationInterface anim (xf);
	config.ConfigureAttributes ();

	m_timeStart = clock();

	//Config::Connect ("/NodeList/0/DeviceList/0/$ns3::dot11s::HwmpProtocol/RouteDiscoveryTime",
	//	MakeCallback(&MeshDot11sSim::RouteDiscoveryTimeSniffer, this));

	Simulator::Stop (Seconds (m_totalTime));
	Simulator::Run ();
	FlowMonitoring();
}

int main (int argc, char *argv[])
{
	MeshDot11sSim2 sim;
	ns3::PacketMetadata::Enable ();

	sim.RunSim(argc, argv);

	return 0;
}

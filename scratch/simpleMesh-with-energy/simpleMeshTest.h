#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mesh-module.h"
#include "ns3/mobility-module.h"
#include "ns3/mesh-helper.h"
#include "ns3/aodv-module.h"

#include "ns3/point-to-point-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"
#include "ns3/config-store-module.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

using namespace ns3;

class MeshDot11sSim
{
    public:
	// Init test
	MeshDot11sSim ();

    //private:
	uint32_t m_xSize; //x size of the grid
	uint32_t m_ySize; //y size of the grid
	double   m_step;  //separation between nodes
	double   m_randomStart;
	double   m_totalTime;
	uint16_t m_packetSize;
	uint32_t m_nIfaces;
	bool     m_chan;
	std::string m_txrate;
	std::string m_root;
	bool     m_showRtable;

	//to calculate the lenght of the simulation
	float m_timeTotal, m_timeStart, m_timeEnd;

	// List of network nodes
	NodeContainer meshNodes;
	NodeContainer staNodes;
	Ptr<Node>     *n;
	Ptr<Node>     bgr;

	// List of all mesh point devices
	NetDeviceContainer meshDevices;
	NetDeviceContainer p2pDevices;

	// MeshHelper. Report is not static methods
	MeshHelper mesh;

    //private:
	// Run test
	virtual void RunSim (int argc, char **argv);

	// Configure test from command line arguments
	virtual void Configure (int argc, char ** argv);

	// Create nodes and setup their mobility
	void CreateTopologyNodes ();

	// Configure Mesh Network layer
	void ConfigureMeshLayer ();

	// Install internet stack on nodes
	void InstallInternetStack ();

	// Install applications randomly
	void SetUpUdpApplication();
	void SetUpTcpApplication(Ptr <Node>, Ptr <Node>, std::string);

	//void showHwmpRoutingTables(Ptr <Node>);
	void showHwmpRoutingTables();

	// Print mesh devices diagnostics
	void FlowMonitoring();
	FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor;

	// report throughput, delay, and pdf
	void Report ();
	void RouteDiscoveryTimeSniffer (std::string context, Time time);
};

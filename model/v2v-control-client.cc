/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 University of Athens (UOA)
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
 * Author:  - Lampros Katsikas <lkatsikas@di.uoa.gr>
 *          - Konstantinos Chatzikokolakis <kchatzi@di.uoa.gr>
 */

#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/string.h"
#include "ns3/packet.h"
#include "ns3/address.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/simulator.h"
#include "ns3/udp-socket.h"
#include "ns3/address-utils.h"
#include "ns3/socket-factory.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/trace-source-accessor.h"
#include "v2v-control-client.h"

#include "ns3/random-variable.h"


namespace ns3 {

static const std::string
ClusterStatusName[V2vControlClient::CLUSTER_STATES] =
{
    "CLUSTER_INITIALIZATION",
    "CLUSTER_HEAD_ELECTION",
    "CLUSTER_FORMATION",
    "CLUSTER_UPDATE"
};

static const std::string & ToString (V2vControlClient::NodeStatus status){
    return ClusterStatusName[status];
}

static const std::string
IncidentName[V2vClusterSap::INCIDENT_STATES] =
{
    "EMERGENCY_EVENT",
    "NOTIFICATION_EVENT"
};

static const std::string & ToString (V2vClusterSap::IncidentType incidentType){
    return IncidentName[incidentType];
}

static const std::string
DegreeName[V2vClusterSap::DEGREE_STATES] =
{
    "STANDALONE",
    "CH",
    "CM"
};

static const std::string & ToString (V2vClusterSap::NodeDegree nodeDegree){
    return DegreeName[nodeDegree];
}

NS_LOG_COMPONENT_DEFINE ("V2vControlClient");
NS_OBJECT_ENSURE_REGISTERED (V2vControlClient);

TypeId V2vControlClient::GetTypeId(void) {
	static TypeId tid =
			TypeId("ns3::V2vControlClient").SetParent<Application>()
			.AddConstructor<V2vControlClient>()
            .AddAttribute("ListeningLocal",
					"The Address on which to Bind the rx socket.",
                    AddressValue(), MakeAddressAccessor(&V2vControlClient::m_peerListening),
					MakeAddressChecker())
            .AddAttribute("ProtocolListeningLocal",
					"The type id of the protocol to use for the rx socket.",
					TypeIdValue(UdpSocketFactory::GetTypeId()),
                    MakeTypeIdAccessor(&V2vControlClient::m_tidListening),
					MakeTypeIdChecker())
            .AddTraceSource("RxLocal", "A packet has been received",
                    MakeTraceSourceAccessor(&V2vControlClient::m_rxTrace))

            .AddAttribute("IncidentWindow",
                    "The incident time window", DoubleValue(4),
                    MakeDoubleAccessor(&V2vControlClient::m_incidentWindow),
                    MakeDoubleChecker<double>())
            .AddAttribute("ClusterTimeMetric",
                    "The maximun size of the TDMA window", DoubleValue(0.5),
                    MakeDoubleAccessor(&V2vControlClient::m_clusterTimeMetric),
                    MakeDoubleChecker<double>())
            .AddAttribute("MinimumTdmaSlot",
                    "The maximun size of the TDMA window", DoubleValue(0.001),
                    MakeDoubleAccessor(&V2vControlClient::m_minimumTdmaSlot),
                    MakeDoubleChecker<double>())
            .AddAttribute("MaxUes",
                    "The maximun size of ues permitted", UintegerValue(100),
                    MakeUintegerAccessor(&V2vControlClient::m_maxUes),
                    MakeUintegerChecker<uint32_t>(1))
			.AddAttribute("PacketSize",
					"The size of packets sent in on state", UintegerValue(512),
                    MakeUintegerAccessor(&V2vControlClient::m_pktSize),
					MakeUintegerChecker<uint32_t>(1))
            .AddAttribute ("TimeWindow",
                    "The time to wait between packets", DoubleValue (1.0),
                    MakeDoubleAccessor (&V2vControlClient::m_timeWindow),
                    MakeDoubleChecker<double>())
            .AddAttribute ("Interval",
                    "The time to wait between packets", TimeValue (Seconds (1.0)),
                    MakeTimeAccessor (&V2vControlClient::m_interval),
                    MakeTimeChecker ())
            .AddAttribute("SendingLocal",
					"The address of the destination", AddressValue(),
                    MakeAddressAccessor(&V2vControlClient::m_peer),
					MakeAddressChecker())
            .AddAttribute("ProtocolSendingLocal",
					"The type of protocol for the tx socket.",
					TypeIdValue(UdpSocketFactory::GetTypeId()),
                    MakeTypeIdAccessor(&V2vControlClient::m_tid),
					MakeTypeIdChecker())
			.AddAttribute ("MobilityModel",
				    "The mobility model of the node.",
				    PointerValue (),
                    MakePointerAccessor (&V2vControlClient::m_mobilityModel),
                    MakePointerChecker<V2vMobilityModel> ())
            .AddTraceSource("TxLocal","A new packet is created and is sent",
                    MakeTraceSourceAccessor(&V2vControlClient::m_txTrace));
	return tid;
}


// Public Members
V2vControlClient::V2vControlClient () {
	NS_LOG_FUNCTION (this);

    m_socket = 0;
    m_socketIncident = 0;
    m_socketListening = 0;

    m_overalDelay = 0;
    m_sentCounter = 0;
    m_changesCounter = 0;
    m_incidentCounter = 0;
    m_formationCounter = 0;

    m_sendEvent = EventId ();
    m_chElectionEvent = EventId ();
}

V2vControlClient::~V2vControlClient () {
	NS_LOG_FUNCTION (this);

    m_socket = 0;
    m_socketIncident = 0;
    m_socketListening = 0;

    m_overalDelay = 0;
    m_sentCounter = 0;
    m_changesCounter = 0;
    m_incidentCounter = 0;
    m_formationCounter = 0;
}

void
V2vControlClient::PrintStatistics (std::ostream &os){
    if(m_incidentCounter == 0){
       m_incidentCounter = 1;       // Avoid division with zero
    }

    os << "***********************" << std::endl
       << "  - Cluster Metrics -  " << std::endl
       << "Node:" << m_currentMobility.imsi << " Sent overal: " << m_sentCounter << " Packets." << std::endl
       << " Formation Messages: " << m_formationCounter << std::endl
       << " Status Changes: " << m_changesCounter << std::endl
       << "-----------------------" << std::endl
       << "  - Insident Metrics -  " << std::endl
       << "Mean delay of incidents delivered: " << (double) m_overalDelay/m_incidentCounter << std::endl
       << "***********************" << std::endl;
}

// Protected Members
void
V2vControlClient::DoDispose (void) {
	NS_LOG_FUNCTION (this);

    m_socket = 0;
    m_socketListening = 0;

	// chain up
	Application::DoDispose();
}

void
V2vControlClient::StartApplication (void)
{
    NS_LOG_FUNCTION (this);
    m_status = V2vControlClient::CLUSTER_INITIALIZATION;

	// Create the socket if not already
	if (!m_socket) {
		m_socket = Socket::CreateSocket(GetNode(), m_tid);
		if (Inet6SocketAddress::IsMatchingType(m_peer)) {
			m_socket->Bind6();
		} else if (InetSocketAddress::IsMatchingType(m_peer)
				|| PacketSocketAddress::IsMatchingType(m_peer)) {
			m_socket->Bind();
		}
		m_socket->Connect(m_peer);
		m_socket->SetAllowBroadcast(true);
		m_socket->ShutdownRecv();

		m_socket->SetConnectCallback(
                MakeCallback(&V2vControlClient::ConnectionSucceeded, this),
                MakeCallback(&V2vControlClient::ConnectionFailed, this));
    }

    if(m_maxUes > 100){
        NS_FATAL_ERROR("Error: Maximum number of ues is 100.");
    }

    StartListeningLocal();
    ScheduleTransmit (Seconds (m_timeWindow));
    AcquireMobilityInfo();
}

void
V2vControlClient::StartListeningLocal (void)    // Called at time specified by Start
{
	NS_LOG_FUNCTION (this);

    m_clusterList.clear();
    m_rStableList.clear();
	m_2rStableList.clear();
	// Create the socket if not already
	if (!m_socketListening) {
		m_socketListening = Socket::CreateSocket(GetNode(), m_tidListening);
		m_socketListening->Bind(m_peerListening);
		m_socketListening->Listen();
		m_socketListening->ShutdownSend();
		if (addressUtils::IsMulticast(m_peerListening)) {
			Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket>(m_socketListening);
			if (udpSocket) {
				// equivalent to setsockopt (MCAST_JOIN_GROUP)
				udpSocket->MulticastJoinGroup(0, m_peerListening);
			} else {
				NS_FATAL_ERROR("Error: joining multicast on a non-UDP socket");
			}
		}
	}

    m_socketListening->SetRecvCallback(MakeCallback(&V2vControlClient::HandleRead, this));
	m_socketListening->SetAcceptCallback(
			MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
            MakeCallback(&V2vControlClient::HandleAccept, this));
	m_socketListening->SetCloseCallbacks(
            MakeCallback(&V2vControlClient::HandlePeerClose, this),
            MakeCallback(&V2vControlClient::HandlePeerError, this));
}

void
V2vControlClient::StopApplication (void) // Called at time specified by Stop
{
	NS_LOG_FUNCTION (this);

	if (m_socket != 0) {
		m_socket->Close();
		m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
		m_socket = 0;
	} else {
		NS_LOG_WARN ("V2vControlClient found null socket to close in StopApplication");
	}
	Simulator::Cancel (m_sendEvent);
    Simulator::Cancel (m_sendIncidentEvent);
    StopListeningLocal();
    PrintStatistics(std::cout);
}

void
V2vControlClient::StopListeningLocal (void)     // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);
  if (m_socketListening)
    {
	  m_socketListening->Close ();
	  m_socketListening->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
	  m_socketListening = 0;
    }
}

Ptr<Socket>
V2vControlClient::GetListeningSocket (void) const {
    NS_LOG_FUNCTION (this);
    return m_socketListening;
}

Ptr<Socket>
V2vControlClient::GetSocket (void) const {
    NS_LOG_FUNCTION (this);
    return m_socket;
}


// Private Members
void
V2vControlClient::HandleRead (Ptr<Socket> socket) {
	NS_LOG_FUNCTION (this << socket);
	Ptr<Packet> packet;
	Address from;
	while ((packet = socket->RecvFrom(from))) {
		if (packet->GetSize() == 0) { //EOF
			break;
		}

		PacketMetadata::ItemIterator metadataIterator = packet->BeginItem();
		PacketMetadata::Item item;
		while (metadataIterator.HasNext()){
		    item = metadataIterator.Next();
		    if(item.tid.GetName() == "ns3::V2vClusterInfoHeader"){

                V2vClusterSap::V2vClusterSap::NeighborInfo otherInfo;
                V2vClusterInfoHeader clusterInfo;
		    	packet->RemoveHeader(clusterInfo);
                otherInfo = clusterInfo.GetMobilityInfo();

                //!< Update rStable List
                std::map<uint64_t, V2vClusterSap::NeighborInfo>::iterator itr = m_rStableList.find(otherInfo.imsi);
                if(IsStable(otherInfo.velocity)){
                    if(itr == m_rStableList.end()){
                        NS_LOG_DEBUG("[HandleRead] => Node:" << m_currentMobility.imsi << " Insert packet:" << otherInfo.imsi);
                        m_rStableList.insert(std::map<uint64_t, V2vClusterSap::NeighborInfo>::value_type(otherInfo.imsi, otherInfo));
                    }
                    else{
                        itr->second = otherInfo;
                    }
                }

                //!< Update 2rStable and cluster List
                std::map<uint64_t, V2vClusterSap::NeighborInfo>::iterator it2r = m_2rStableList.find(otherInfo.imsi);
                if(it2r == m_2rStableList.end()){
                    NS_LOG_DEBUG("[HandleRead] => Node:" << m_currentMobility.imsi << " Insert packet:" << otherInfo.imsi);
                    m_2rStableList.insert(std::map<uint64_t, V2vClusterSap::NeighborInfo>::value_type(otherInfo.imsi, otherInfo));
                }
                else{
                    it2r->second = otherInfo;
                }

                if(m_status == V2vControlClient::CLUSTER_UPDATE){

                    if(m_currentMobility.degree == V2vClusterSap::CH){
                        if(otherInfo.clusterId == m_currentMobility.imsi){
                            std::map<uint64_t, V2vClusterSap::NeighborInfo>::iterator itc = m_clusterList.find(otherInfo.imsi);
                            if(itc == m_clusterList.end()){
                                NS_LOG_DEBUG("[HandleRead] => Node:" << m_currentMobility.imsi << " - insert node:" << otherInfo.imsi);
                                m_clusterList.insert(std::map<uint64_t, V2vClusterSap::NeighborInfo>::value_type(otherInfo.imsi, otherInfo));
                            }
                            else{
                                //!< Update UE Info
                                NS_LOG_DEBUG("[HandleRead] => Node:" << m_currentMobility.imsi << " - update node:" << otherInfo.imsi);
                                itc->second = otherInfo;
                            }
                        }
                        else{
                            //!< Check Cluster Merge
                            if(m_clusterList.size() == 0){
                                uint64_t potentialCH = MergeCheck();

                                if(m_rStableList.count(potentialCH) > 0){
                                    V2vClusterSap::NeighborInfo potential = m_rStableList.find(potentialCH)->second;

                                    if(m_currentMobility.imsi < potential.imsi){
                                        NS_LOG_DEBUG("[HandleRead] => Node:" << m_currentMobility.imsi << " - merge with node:" << potential.imsi);
                                        m_currentMobility.degree = V2vClusterSap::CM;
                                        m_currentMobility.clusterId = potential.imsi;
                                        m_changesCounter ++;

                                        RemoveIncidentSocket ();
                                        CreateIncidentSocket (from);
                                    }
                                }
                            }
                        }
                    }
                    else if(m_currentMobility.degree == V2vClusterSap::STANDALONE){
                        uint64_t potentialCH = MergeCheck();
                        if(m_rStableList.count(potentialCH) > 0){
                            V2vClusterSap::NeighborInfo potential = m_rStableList.find(potentialCH)->second;

                            NS_LOG_DEBUG("[HandleRead] => Node:" << m_currentMobility.imsi
                                          << " - Attach to new CH node:" << potential.imsi);
                            m_currentMobility.degree = V2vClusterSap::CM;
                            m_currentMobility.clusterId = potential.imsi;
                            m_changesCounter ++;

                            RemoveIncidentSocket ();
                            CreateIncidentSocket (from);
                        }
                        else{
                            NS_LOG_DEBUG("[HandleRead] => To Become new CH: " << m_currentMobility.imsi);

                            NS_LOG_DEBUG("Node Status: " << ToString(m_status));
                            m_currentMobility.degree = V2vClusterSap::CH;
                            m_currentMobility.clusterId = m_currentMobility.imsi;
                            m_changesCounter ++;
                        }
                    }
                }

				if (InetSocketAddress::IsMatchingType(from)) {
                    NS_LOG_INFO ("[HandleRead] => At time " << Simulator::Now ().GetSeconds ()
                            << "s node ["<< m_currentMobility.imsi <<"] received "
							<< packet->GetSize () << " bytes from "
							<< InetSocketAddress::ConvertFrom(from).GetIpv4 ()
							<< " port " << InetSocketAddress::ConvertFrom (from).GetPort ()
                            << " seq: " << clusterInfo.GetSeq ()
                            << " degree: " << ToString (otherInfo.degree)
                            << " position: " << otherInfo.position
                            << " - Velocity: " << otherInfo.velocity
                            << " - Direction: " << otherInfo.direction);
				} else if (Inet6SocketAddress::IsMatchingType(from)) {
                    NS_LOG_INFO ("[HandleRead] => At time " << Simulator::Now ().GetSeconds ()
                            << "s node ["<< m_currentMobility.imsi <<"] received "
							<< packet->GetSize () << " bytes from "
							<< InetSocketAddress::ConvertFrom(from).GetIpv4 ()
							<< " port " << InetSocketAddress::ConvertFrom (from).GetPort ()
                            << " seq: " << clusterInfo.GetSeq ()
                            << " degree: " << ToString (otherInfo.degree)
                            << " position: " << otherInfo.position
                            << " - Velocity: " << otherInfo.velocity
                            << " - Direction: " << otherInfo.direction);
                }
		    }
		    else if(item.tid.GetName() == "ns3::V2vInitiateClusterHeader"){

                if(m_status == V2vControlClient::CLUSTER_INITIALIZATION){
                    m_status = V2vControlClient::CLUSTER_HEAD_ELECTION;

					if (InetSocketAddress::IsMatchingType(from)) {
                        NS_LOG_INFO ("[HandleRead] => At time " << Simulator::Now ().GetSeconds ()
                                << "s node ["<< m_currentMobility.imsi <<"] received "
								<< packet->GetSize () << " bytes from "
								<< InetSocketAddress::ConvertFrom(from).GetIpv4 ()
								<< " port " << InetSocketAddress::ConvertFrom (from).GetPort ()
								<< " to check for CH suitability.");
					} else if (Inet6SocketAddress::IsMatchingType(from)) {
                        NS_LOG_INFO ("[HandleRead] => At time " << Simulator::Now ().GetSeconds ()
                                << "s node ["<< m_currentMobility.imsi <<"] received "
								<< packet->GetSize () << " bytes from "
								<< InetSocketAddress::ConvertFrom(from).GetIpv4 ()
								<< " port " << InetSocketAddress::ConvertFrom (from).GetPort ()
								<< " to check for CH suitability.");
					}

					//!< Parse V2vInitiateClusterHeader Info
					V2vInitiateClusterHeader initiateCluster;
					packet->RemoveHeader (initiateCluster);
                    m_currentMobility.clusterId = initiateCluster.GetClusterId();
                    std::map<uint64_t, V2vClusterSap::NeighborInfo>::iterator foundIt = m_2rStableList.find(initiateCluster.GetClusterId());
                    if( (foundIt != m_2rStableList.end()) ){

                        //!< Suitability check [should be applied to r-distance neighbors]
                        double waitingTime = SuitabilityCheck();
                        NS_LOG_UNCOND ("[HandleRead] => NodeId: " << m_currentMobility.imsi << " WaitingTime is: " << waitingTime);

                        //!< Handle chElection Event
                        m_chElectionEvent = Simulator::Schedule (Seconds(waitingTime), &V2vControlClient::FormCluster, this);

                        UniformVariable randomIncident ((int)waitingTime + 1, m_incidentWindow);
                        ScheduleIncidentEvent (Seconds (randomIncident.GetValue ()));
					}
					else{
                        /// Not in 2rStableList
                        m_status = V2vControlClient::CLUSTER_INITIALIZATION;
					}
		    	}
		    	else{
                    //!< Process only the first request and ignore the rest
                    NS_LOG_DEBUG ("[HandleRead] => NodeId: " << m_currentMobility.imsi << " Ignore further requests for CH suitability...");
		    	}
		    }
		    else if(item.tid.GetName() == "ns3::V2vFormClusterHeader"){

                NS_ASSERT( (m_status != V2vControlClient::CLUSTER_HEAD_ELECTION) || (m_status != V2vControlClient::CLUSTER_FORMATION));

                V2vFormClusterHeader formCluster;
                packet->RemoveHeader (formCluster);
                std::map<uint64_t, V2vClusterSap::NeighborInfo>::iterator it = m_2rStableList.find(formCluster.GetMobilityInfo().clusterId);

                if(it != m_2rStableList.end()){
                    if(m_status == V2vControlClient::CLUSTER_HEAD_ELECTION){

                        m_status = V2vControlClient::CLUSTER_FORMATION;
                        m_chElectionEvent.Cancel();
                        NS_LOG_DEBUG ("[HandleRead] => NodeId: " << m_currentMobility.imsi
                                      << " connected to cluster: " << formCluster.GetMobilityInfo().clusterId);

                        //!< Apply received CH info
                        m_status = V2vControlClient::CLUSTER_UPDATE;
                        m_currentMobility.degree = V2vClusterSap::CM;
                        m_currentMobility.clusterId = formCluster.GetMobilityInfo().clusterId;
                        ScheduleTransmit (Seconds((m_timeWindow)));

                        double updateTime = (int) Simulator::Now ().GetSeconds () + 1.5;
                        Simulator::Schedule(Seconds(updateTime-Simulator::Now ().GetSeconds ()), &V2vControlClient::UpdateNeighborList, this);

                        CreateIncidentSocket (from);
                    }
                    else if(m_status == V2vControlClient::CLUSTER_FORMATION){
                        NS_LOG_DEBUG ("[HandleRead] => NodeId: " << m_currentMobility.imsi << " Node is already a Cluster Member.");
                    }
                }
                else{
                    /// Not in 2rStableList
                }
            }
            else if(item.tid.GetName() == "ns3::V2vIncidentEventHeader"){

                V2vIncidentEventHeader incidentHeader;
                packet->RemoveHeader (incidentHeader);

                if(m_incidentTimestamp.GetSeconds () == incidentHeader.GetTs().GetSeconds ()){

                    /// Calculate Delay
                    m_overalDelay += Simulator::Now ().GetSeconds () - m_incidentTimestamp.GetSeconds ();
                    NS_LOG_UNCOND ("Node: " << m_currentMobility.imsi << " received back V2vIncidentEventHeader:"
                                  << ". Incident Delay is: " << Simulator::Now ().GetSeconds () - m_incidentTimestamp.GetSeconds () << " Seconds");
                    NS_LOG_UNCOND ("--------------------------------------------------------------------------------------------");
                }

                if( (m_currentMobility.degree == V2vClusterSap::CH) && (m_currentMobility.clusterId == incidentHeader.GetIncidentInfo().clusterId) ){

                    //!< Broadcast Incident to Cluster Members
                    Ptr<Packet> packet = Create<Packet>(0);
                    packet->AddHeader(incidentHeader);

                    m_socket->Send(packet);
                    if (InetSocketAddress::IsMatchingType(m_peer)) {
                        NS_LOG_UNCOND ("[Send] Broadcast Incident Message from " << m_currentMobility.imsi << "=> At time " << Simulator::Now ().GetSeconds ()
                                <<" sent " << packet->GetSize () << " bytes to "
                                << InetSocketAddress::ConvertFrom(m_peer).GetIpv4 ()
                                << " port " << InetSocketAddress::ConvertFrom (m_peer).GetPort ()
                                << " - Event Type is:" << ToString (incidentHeader.GetIncidentInfo ().incidentType));
                    } else if (Inet6SocketAddress::IsMatchingType(m_peer)) {
                        NS_LOG_UNCOND ("[Send] Broadcast Incident Message from " << m_currentMobility.imsi << "=> At time " << Simulator::Now ().GetSeconds ()
                                <<" sent " << packet->GetSize () << " bytes to "
                                << packet->GetSize () << " bytes to "
                                << Inet6SocketAddress::ConvertFrom(m_peer).GetIpv6 ()
                                << " port " << Inet6SocketAddress::ConvertFrom (m_peer).GetPort ()
                                << " - Event Type is:" << ToString (incidentHeader.GetIncidentInfo ().incidentType));
                    }
                }

            }
            m_rxTrace(packet, from);
        }
    }
}

void
V2vControlClient::CreateIncidentSocket (Address from)
{
    NS_LOG_FUNCTION (this);

    //!< Create p2p socket with ClusterHead for incident event transmission
    Ipv4Address chAddress = InetSocketAddress::ConvertFrom(from).GetIpv4 ();
    uint16_t chPort = InetSocketAddress::ConvertFrom (m_peer).GetPort ();
    m_peerIncident = Address(InetSocketAddress(chAddress, chPort));

    // Create the socket if not already
    if (!m_socketIncident) {
        m_socketIncident = Socket::CreateSocket(GetNode(), m_tid);
        if (Inet6SocketAddress::IsMatchingType(m_peerIncident)) {
            m_socketIncident->Bind6();
        } else if (InetSocketAddress::IsMatchingType(m_peerIncident)
                || PacketSocketAddress::IsMatchingType(m_peerIncident)) {
            m_socketIncident->Bind();
        }
        m_socketIncident->Connect(m_peerIncident);
        m_socketIncident->SetAllowBroadcast(false);
        m_socketIncident->ShutdownRecv();

        m_socketIncident->SetConnectCallback(
                MakeCallback(&V2vControlClient::ConnectionCHSucceeded, this),
                MakeCallback(&V2vControlClient::ConnectionCHFailed, this));
    }
}

void
V2vControlClient::RemoveIncidentSocket (void)
{
    NS_LOG_FUNCTION (this);

    if (m_socketIncident != 0) {
        m_socketIncident->Close();
        m_socketIncident->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
        m_socketIncident = 0;
    } else {
        NS_LOG_WARN ("m_socketIncident null socket to close...");
    }
}

void
V2vControlClient::ConnectionCHSucceeded (Ptr<Socket> socket) {
    NS_LOG_FUNCTION (this << socket);
    NS_LOG_DEBUG ("P2P Connection with CH Successful");
}

void
V2vControlClient::ConnectionCHFailed (Ptr<Socket> socket) {
    NS_LOG_FUNCTION (this << socket);
    NS_FATAL_ERROR("Error: joining CH socket");
}

uint64_t
V2vControlClient::MergeCheck (void){
    uint64_t id = 0;
    double r = 80;              //!< transmition range
    double rt = 0.0;            //!< Suitability metric for CH  selection
    double boundary = 0.0;
    for(std::map<uint64_t, V2vClusterSap::NeighborInfo>::iterator itSearch = m_rStableList.begin(); itSearch != m_rStableList.end(); ++itSearch){
        V2vClusterSap::NeighborInfo node = itSearch->second;
        if(node.degree == V2vClusterSap::CH){
            if( ((m_currentMobility.position.x < node.position.x) && (m_currentMobility.velocity.x > 0) && fabs(m_currentMobility.velocity.x) < fabs(node.velocity.x))
                    || ((m_currentMobility.position.x < node.position.x) && (m_currentMobility.velocity.x < 0) && fabs(m_currentMobility.velocity.x) > fabs(node.velocity.x)) ){
                rt = (r-fabs(m_currentMobility.position.x - node.position.x)) / (fabs(m_currentMobility.velocity.x-node.velocity.x));
                NS_LOG_DEBUG ("[MergeCheck] => NODES ARE GETTING FAR AND FAR - RT:" << rt << "current Node:" << m_currentMobility.imsi << " - with node:" << node.imsi);
            }
            if( ((m_currentMobility.position.x < node.position.x) && (m_currentMobility.velocity.x > 0) && fabs(m_currentMobility.velocity.x) > fabs(node.velocity.x))
                    || ((m_currentMobility.position.x < node.position.x) && (m_currentMobility.velocity.x < 0) && fabs(m_currentMobility.velocity.x) < fabs(node.velocity.x)) ){
                rt = (r+fabs(m_currentMobility.position.x - node.position.x)) / (fabs(m_currentMobility.velocity.x-node.velocity.x));
                NS_LOG_DEBUG("[MergeCheck] => NODES ARE GETTING CLOSER AND CLOSER - RT:" << rt << "current Node:" << m_currentMobility.imsi << " - with node:" << node.imsi);
            }

            if( ((m_currentMobility.position.x > node.position.x) && (m_currentMobility.velocity.x > 0) && fabs(m_currentMobility.velocity.x) > fabs(node.velocity.x))
                    || ((m_currentMobility.position.x > node.position.x) && (m_currentMobility.velocity.x < 0) && fabs(m_currentMobility.velocity.x) < fabs(node.velocity.x)) ){
                rt = (r-fabs(m_currentMobility.position.x - node.position.x)) / (fabs(m_currentMobility.velocity.x-node.velocity.x));
                NS_LOG_DEBUG("[MergeCheck] => NODES ARE GETTING FAR AND FAR - RT:" << rt << "current Node:" << m_currentMobility.imsi << " - with node:" << node.imsi);
            }
            if( ((m_currentMobility.position.x > node.position.x) && (m_currentMobility.velocity.x > 0) && fabs(m_currentMobility.velocity.x) < fabs(node.velocity.x))
                    || ((m_currentMobility.position.x > node.position.x) && (m_currentMobility.velocity.x < 0) && fabs(m_currentMobility.velocity.x) > fabs(node.velocity.x)) ){
                rt = (r+fabs(m_currentMobility.position.x - node.position.x)) / (fabs(m_currentMobility.velocity.x-node.velocity.x));
                NS_LOG_DEBUG("[MergeCheck] => NODES ARE GETTING CLOSER AND CLOSER - RT:" << rt << "current Node:" << m_currentMobility.imsi << " - with node:" << node.imsi);
            }

            if(rt > boundary){
                id = itSearch->first;
                boundary = rt;
            }
        }
    }
    NS_LOG_DEBUG ("[MergeCheck] => Returned Id is: " << id << " - with Remaining Time(RT):" << boundary);
    return id;
}

void V2vControlClient::AcquireMobilityInfo (void){

    //!< Acquire current mobility stats
    m_currentMobility.ts = Simulator::Now ();
    m_currentMobility.imsi = this->GetNode()->GetId();
    m_currentMobility.position = m_mobilityModel->GetPosition();
    m_currentMobility.velocity = m_mobilityModel->GetVelocity();
    m_currentMobility.direction = m_mobilityModel->GetDirection();
}

double
V2vControlClient::SuitabilityCheck (void){

    double size = m_rStableList.size();
    //!< Find mean value of position and velocity
    if(m_rStableList.size() == 0){
        return m_currentMobility.imsi;
    }

    Vector p,v;
    for(std::map<uint64_t, V2vClusterSap::NeighborInfo>::iterator it = m_rStableList.begin(); it != m_rStableList.end(); ++it) {
        uint64_t key = it->first;
        NS_ASSERT(key != m_currentMobility.imsi);

        V2vClusterSap::NeighborInfo value = it->second;
        p.x += value.position.x;
        p.y += value.position.y;

        v.x += value.velocity.x;
        v.y += value.velocity.y;
    }
    p.x = p.x/size;
    p.y = p.y/size;
    v.x = v.x/size;
    v.y = v.y/size;

    NS_LOG_DEBUG("[SuitabilityCheck] => Mean p.x = " << p.x << " - Mean p.y = " << p.y);
    NS_LOG_DEBUG("[SuitabilityCheck] => Mean v.x = " << v.x << " - Mean v.y = " << v.y);


    //!< Find standard deviation of position and velocity
    Vector ps,vs;
    for(std::map<uint64_t, V2vClusterSap::NeighborInfo>::iterator it = m_rStableList.begin(); it != m_rStableList.end(); ++it) {
        uint64_t key = it->first;
        NS_ASSERT(key != m_currentMobility.imsi);

        V2vClusterSap::NeighborInfo value = it->second;
        ps.x += pow((value.position.x - p.x), 2.0);
        ps.y += pow((value.position.y - p.y), 2.0);
        vs.x += pow((value.velocity.x - v.x), 2.0);
        vs.y += pow((value.velocity.y - v.y), 2.0);
    }
    ps.x = ps.x/size;
    ps.y = ps.y/size;
    vs.x = vs.x/size;
    vs.y = vs.y/size;

    NS_LOG_DEBUG("[SuitabilityCheck] => ps.x = " << ps.x << " - ps.y = " << ps.y);
    NS_LOG_DEBUG("[SuitabilityCheck] => vs.x = " << vs.x << " - vs.y = " << vs.y);

    /// Avoid division with zero if vehicle moves horizontally or vertiacally
    Vector pNorm;
    if(ps.x == 0){
        pNorm.x = (m_currentMobility.position.x - p.x)/1;
    }
    else{
        pNorm.x = (m_currentMobility.position.x - p.x)/ps.x;
    }

    if(ps.y == 0){
        pNorm.y = (m_currentMobility.position.y - p.y)/1;
    }
    else{
        pNorm.y = (m_currentMobility.position.y - p.y)/ps.y;
    }


    Vector vNorm;
    if(vs.x == 0){
        vNorm.x = (m_currentMobility.velocity.x - v.x)/1;
    }
    else{
        vNorm.x = (m_currentMobility.velocity.x - v.x)/vs.x;
    }


    if(vs.y == 0){
        vNorm.y = (m_currentMobility.velocity.y - v.y)/1;
    }
    else{
        vNorm.y = (m_currentMobility.velocity.y - v.y)/vs.y;
    }
    NS_LOG_DEBUG("[SuitabilityCheck] => pNorm.x:" << pNorm.x << " - vNorm.x:" << vNorm.x);
    NS_LOG_DEBUG("[SuitabilityCheck] => pNorm.y:" << pNorm.y << " - vNorm.y:" << vNorm.y);

    double w = fabs(pNorm.x) + fabs(pNorm.y) + fabs(vNorm.x) + fabs(vNorm.y);

    NS_LOG_DEBUG("[SuitabilityCheck] => w = " << w << " - u = "
                 << (double)size* exp((-m_clusterTimeMetric)* w) << " - size:" << size);

    return (double)size* exp((-m_clusterTimeMetric)* w);
}

void
V2vControlClient::FormCluster (void){
    m_status = V2vControlClient::CLUSTER_FORMATION;
    ScheduleTransmit (Seconds(0.));
}

void
V2vControlClient::StatusReport (void){

    NS_LOG_UNCOND("\n\n-----------------------------------------------------------------------------");
    NS_LOG_UNCOND ("[StatusReport] => At time " << Simulator::Now ().GetSeconds ()
                   << "s node ["<< m_currentMobility.imsi << "] is: " << ToString (m_currentMobility.degree)
        << " in Cluster: " << m_currentMobility.clusterId
        << " having  ===> \n position: " << m_currentMobility.position << " - Velocity: " << m_currentMobility.velocity
        << " - Direction: " << m_currentMobility.direction
        << "\n last packet sent:" << m_currentMobility.ts<< "s"
        << "\n Neighbors: " << m_2rStableList.size());
    NS_LOG_UNCOND("----------------------------  2rStableList  ---------------------------------");
    for(std::map<uint64_t, V2vClusterSap::NeighborInfo>::iterator it = m_2rStableList.begin(); it != m_2rStableList.end(); ++it){
        uint64_t id = it->first;
        V2vClusterSap::NeighborInfo node = it->second;
        NS_LOG_UNCOND(" * key: " << id << " clusterId: " << node.clusterId
                << " Degree:" << ToString (node.degree) << " Imsi:" << node.imsi
                << " Position:" << node.position << " Velocity" << node.velocity
                << " Direction:" << node.direction
                << " last packet sent:" << node.ts<< "s");
    }
    NS_LOG_UNCOND("-----------------------------  rStableList  ---------------------------------");
    for(std::map<uint64_t, V2vClusterSap::NeighborInfo>::iterator it = m_rStableList.begin(); it != m_rStableList.end(); ++it){
        uint64_t id = it->first;
        V2vClusterSap::NeighborInfo node = it->second;
        NS_LOG_UNCOND(" * key: " << id << " clusterId: " << node.clusterId
                << " Degree:" << ToString (node.degree)
                << " Imsi:" << node.imsi
                << " Position:" << node.position << " Velocity" << node.velocity
                << " Direction:" << node.direction);
    }
    NS_LOG_UNCOND("-----------------------------  clusterList  ---------------------------------");
    for(std::map<uint64_t, V2vClusterSap::NeighborInfo>::iterator it = m_clusterList.begin(); it != m_clusterList.end(); ++it){
        uint64_t id = it->first;
        V2vClusterSap::NeighborInfo node = it->second;
        NS_LOG_UNCOND(" * key: " << id << " clusterId: " << node.clusterId
                << " Degree:" << ToString (node.degree)
                << " Imsi:" << node.imsi
                << " Position:" << node.position << " Velocity" << node.velocity
                << " Direction:" << node.direction);
    }
}

void
V2vControlClient::HandleAccept (Ptr<Socket> s, const Address& from) {
	NS_LOG_FUNCTION (this << s << from);
    s->SetRecvCallback(MakeCallback(&V2vControlClient::HandleRead, this));
}

void
V2vControlClient::HandlePeerClose (Ptr<Socket> socket) {
	NS_LOG_FUNCTION (this << socket);
}

void
V2vControlClient::HandlePeerError (Ptr<Socket> socket) {
	NS_LOG_FUNCTION (this << socket);
}



void
V2vControlClient::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_sendEvent = Simulator::Schedule (dt, &V2vControlClient::Send, this);
  NS_LOG_DEBUG("[ScheduleTransmit] => NodeId:" << m_currentMobility.imsi << " EventInfo:"
               << m_sendEvent.GetTs() << " status: " << ToString(m_status));
}

void
V2vControlClient::Send (void) {
	NS_LOG_FUNCTION (this);
    NS_LOG_DEBUG("[Send] => NodeId:" << m_currentMobility.imsi << " EventInfo:"
                 << m_sendEvent.GetTs() << " status: " << ToString(m_status));
	NS_ASSERT(m_sendEvent.IsExpired());

	switch (m_status) {
        case V2vControlClient::CLUSTER_INITIALIZATION:{

			V2vClusterInfoHeader clusterInfo;
            clusterInfo.SetSeq(m_sentCounter);
            clusterInfo.SetMobilityInfo(m_currentMobility);

            Ptr<Packet> packet = Create<Packet>(0);
            packet->AddHeader(clusterInfo);
			m_txTrace(packet);
			m_socket->Send(packet);
			++ m_sentCounter;
            m_formationCounter ++;
			if (InetSocketAddress::IsMatchingType(m_peer)) {
                NS_LOG_INFO ("[Send] CLUSTER_INITIALIZATION => At time " << Simulator::Now ().GetSeconds ()
                        << "s node[IMSI] ["<< m_currentMobility.imsi <<"] sent "
						<< packet->GetSize () << " bytes to "
						<< InetSocketAddress::ConvertFrom(m_peer).GetIpv4 ()
						<< " port " << InetSocketAddress::ConvertFrom (m_peer).GetPort ()
                        << " bytes."
                        << " position: " << m_currentMobility.position
                        << " - Velocity: " << m_currentMobility.velocity
                        << " - Direction: " << m_currentMobility.direction);
			} else if (Inet6SocketAddress::IsMatchingType(m_peer)) {
                NS_LOG_INFO ("[Send] CLUSTER_INITIALIZATION => At time " << Simulator::Now ().GetSeconds ()
                        << "s node[IMSI] ["<< m_currentMobility.imsi <<"] sent "
						<< packet->GetSize () << " bytes to "
						<< Inet6SocketAddress::ConvertFrom(m_peer).GetIpv6 ()
						<< " port " << Inet6SocketAddress::ConvertFrom (m_peer).GetPort ()
                        << " bytes."
						<< " position: "
                        << m_currentMobility.position << " - Velocity: " << m_currentMobility.velocity
                        << " - Direction: " << m_currentMobility.direction);
			}
            Simulator::Schedule (Seconds(m_minimumTdmaSlot*m_maxUes), &V2vControlClient::InitiateCluster, this);
			break;
		}
        case V2vControlClient::CLUSTER_HEAD_ELECTION:{

			V2vInitiateClusterHeader initiateCluster;
            initiateCluster.SetSeq(m_sentCounter);
            initiateCluster.SetClusterId(m_currentMobility.imsi);

            Ptr<Packet> packet = Create<Packet>(0);
            packet->AddHeader(initiateCluster);
            m_txTrace(packet);
            m_socket->Send(packet);
            ++ m_sentCounter;
            m_formationCounter ++;
			if (InetSocketAddress::IsMatchingType(m_peer)) {
                NS_LOG_INFO ("[Send] CLUSTER_HEAD_ELECTION => At time " << Simulator::Now ().GetSeconds ()
                        << "s node[IMSI] ["<< m_currentMobility.imsi <<"] sent "
						<< packet->GetSize () << " bytes to "
						<< InetSocketAddress::ConvertFrom(m_peer).GetIpv4 ()
						<< " port " << InetSocketAddress::ConvertFrom (m_peer).GetPort ()
						<< " as a COV");
			} else if (Inet6SocketAddress::IsMatchingType(m_peer)) {
                NS_LOG_INFO ("[Send] CLUSTER_HEAD_ELECTION => At time " << Simulator::Now ().GetSeconds ()
                        << "s node[IMSI] ["<< m_currentMobility.imsi <<"] sent "
						<< packet->GetSize () << " bytes to "
						<< Inet6SocketAddress::ConvertFrom(m_peer).GetIpv6 ()
						<< " port " << Inet6SocketAddress::ConvertFrom (m_peer).GetPort ()
						<< " as a COV");
			}

			double waitingTime = SuitabilityCheck();
            NS_LOG_UNCOND ("[HandleRead] => NodeId: " << m_currentMobility.imsi << " WaitingTime is: " << waitingTime);
            m_chElectionEvent = Simulator::Schedule (Seconds(waitingTime), &V2vControlClient::FormCluster, this);

            UniformVariable randomIncident ((int)waitingTime + 1, m_incidentWindow);
            ScheduleIncidentEvent (Seconds (randomIncident.GetValue ()));

            break;
        }
        case V2vControlClient::CLUSTER_FORMATION:
        {
            AcquireMobilityInfo();
            m_currentMobility.degree = V2vClusterSap::CH;
            m_currentMobility.clusterId = m_currentMobility.imsi;

            V2vFormClusterHeader clusterInfo;
            clusterInfo.SetSeq(m_sentCounter);
            clusterInfo.SetMobilityInfo(m_currentMobility);

            Ptr<Packet> packet = Create<Packet>(0);
            packet->AddHeader(clusterInfo);
            m_txTrace(packet);
			m_socket->Send(packet);
            ++ m_sentCounter;
            m_formationCounter ++;
			if (InetSocketAddress::IsMatchingType(m_peer)) {
                NS_LOG_INFO ("[Send] CLUSTER_FORMATION => At time " << Simulator::Now ().GetSeconds ()
                        << "s node[IMSI] ["<< m_currentMobility.imsi <<"] sent "
						<< packet->GetSize () << " bytes to "
						<< InetSocketAddress::ConvertFrom(m_peer).GetIpv4 ()
						<< " port " << InetSocketAddress::ConvertFrom (m_peer).GetPort ()
                        << " to become CH.");
			} else if (Inet6SocketAddress::IsMatchingType(m_peer)) {
                NS_LOG_INFO ("[Send] CLUSTER_FORMATION => At time " << Simulator::Now ().GetSeconds ()
                        << "s node[IMSI] ["<< m_currentMobility.imsi <<"] sent "
						<< packet->GetSize () << " bytes to "
						<< Inet6SocketAddress::ConvertFrom(m_peer).GetIpv6 ()
						<< " port " << Inet6SocketAddress::ConvertFrom (m_peer).GetPort ()
                        << " to become CH.");
			}

            Simulator::Schedule (Seconds(0.), &V2vControlClient::UpdateNeighbors, this);

            /// Schecule list updating
            double updateTime = (int) Simulator::Now ().GetSeconds () + 1.5;
            Simulator::Schedule(Seconds(updateTime-Simulator::Now ().GetSeconds ()), &V2vControlClient::UpdateNeighborList, this);

			break;
        }
        case V2vControlClient::CLUSTER_UPDATE:{

            if(m_currentMobility.degree == V2vClusterSap::CH){
                StatusReport ();
            }

            AcquireMobilityInfo();
            V2vClusterInfoHeader clusterInfo;
            clusterInfo.SetSeq(m_sentCounter);
            clusterInfo.SetMobilityInfo(m_currentMobility);

            Ptr<Packet> packet = Create<Packet>(0);
            packet->AddHeader(clusterInfo);
            m_txTrace(packet);
            m_socket->Send(packet);
            ++ m_sentCounter;
            if (InetSocketAddress::IsMatchingType(m_peer)) {
                NS_LOG_INFO ("[Send] CLUSTER_UPDATE => Cluster Update At time: " << Simulator::Now ().GetSeconds ()
                        << "s node[IMSI] ["<< m_currentMobility.imsi <<"] sent "
                        << packet->GetSize () << " bytes to "
                        << InetSocketAddress::ConvertFrom(m_peer).GetIpv4 ()
                        << " port " << InetSocketAddress::ConvertFrom (m_peer).GetPort ()
                        << " bytes."
                        << " position: " << m_currentMobility.position
                        << " - Velocity: " << m_currentMobility.velocity
                        << " - Direction: " << m_currentMobility.direction);
            } else if (Inet6SocketAddress::IsMatchingType(m_peer)) {
                NS_LOG_INFO ("[Send] CLUSTER_UPDATE => Cluster Update At time: " << Simulator::Now ().GetSeconds ()
                        << "s node[IMSI] ["<< m_currentMobility.imsi <<"] sent "
                        << packet->GetSize () << " bytes to "
                        << Inet6SocketAddress::ConvertFrom(m_peer).GetIpv6 ()
                        << " port " << Inet6SocketAddress::ConvertFrom (m_peer).GetPort ()
                        << " bytes."
                        << " position: " << m_currentMobility.position
                        << " - Velocity: " << m_currentMobility.velocity
                        << " - Direction: " << m_currentMobility.direction);
            }
            ScheduleTransmit (m_interval);
            break;
        }
		default:
            NS_LOG_DEBUG ("[Send] => Default Case NodeId [Transmit] " << m_currentMobility.imsi
                          << " - Current Status: " << ToString(m_status));
			break;
	}
}

void
V2vControlClient::UpdateNeighbors (void){
    m_status = V2vControlClient::CLUSTER_UPDATE;
    ScheduleTransmit (m_interval);
}

void
V2vControlClient::InitiateCluster (void){
    if(IsSlowestNode()){
        m_status = V2vControlClient::CLUSTER_HEAD_ELECTION;
        ScheduleTransmit(Seconds(m_minimumTdmaSlot*m_maxUes));
	}
}

//!< Check if the neighbor is stable comparing the relative speed
bool
V2vControlClient::IsStable (Vector velocity){

    Vector speed;
    double vTh = 10;
    speed.x = fabs(m_currentMobility.velocity.x - velocity.x);
    speed.y = fabs(m_currentMobility.velocity.y - velocity.y);

    NS_LOG_DEBUG("[IsStable] => vTh is:" << vTh << " - speed difference is:" << speed.x + speed.y);
    if(speed.x+speed.y > vTh){
        return false;
    }
    return true;
}

bool
V2vControlClient::IsSlowestNode (void){
    NS_LOG_DEBUG("Node:" << m_currentMobility.imsi << "has " << m_2rStableList.size() << " list Size");

    //!< Simulate the 2r channel taking care only for distance/2 neighbors
    double maxDistance;
    for(std::map<uint64_t, V2vClusterSap::NeighborInfo>::iterator it = m_2rStableList.begin(); it != m_2rStableList.end(); ++it) {
        V2vClusterSap::NeighborInfo value = it->second;
        if((fabs( m_currentMobility.position.x - value.position.x) + fabs( m_currentMobility.position.y - value.position.y)) > maxDistance){
            maxDistance = fabs( m_currentMobility.position.x - value.position.x) + fabs( m_currentMobility.position.y - value.position.y);
        }
    }

    for(std::map<uint64_t, V2vClusterSap::NeighborInfo>::iterator it = m_2rStableList.begin(); it != m_2rStableList.end(); ++it) {
        uint32_t key = it->first;
        V2vClusterSap::NeighborInfo value = it->second;

        if((fabs( m_currentMobility.position.x - value.position.x) + fabs( m_currentMobility.position.y - value.position.y)) < (maxDistance)/2){
            NS_LOG_DEBUG("Found: " <<  key << " with speed: " << value.velocity.x);

            if(fabs(value.velocity.x + value.velocity.y) < fabs(m_currentMobility.velocity.x + m_currentMobility.velocity.y)){
                return false;
            }
            if(fabs(value.velocity.x + value.velocity.y) == fabs(m_currentMobility.velocity.x + m_currentMobility.velocity.y)){
                if(value.imsi < m_currentMobility.imsi){
                    return false;
                }
            }
        }
    }

    return true;
}

void
V2vControlClient::ConnectionSucceeded (Ptr<Socket> socket) {
    NS_LOG_FUNCTION (this << socket);
}

void
V2vControlClient::ConnectionFailed (Ptr<Socket> socket) {
	NS_LOG_FUNCTION (this << socket);
}

void V2vControlClient::UpdateNeighborList (void){

    //!< Update Neighbor's List according to Timestamps
    for(std::map<uint64_t, V2vClusterSap::NeighborInfo>::iterator it = m_2rStableList.begin(); it != m_2rStableList.end();){
        uint64_t key = it->first;
        V2vClusterSap::NeighborInfo value = it->second;
        if(m_currentMobility.ts.GetSeconds() - value.ts.GetSeconds() > 1.2){
            m_2rStableList.erase(it++);
            NS_LOG_DEBUG ("[UpdateNeighborList] => At: " << Simulator::Now().GetSeconds() << " Node::"
                          << m_currentMobility.imsi << " - Removing Node:" << value.imsi
                          << " with last sent time:" << value.ts.GetSeconds());
            if(value.imsi == m_currentMobility.clusterId){
                NS_LOG_DEBUG ("[UpdateNeighborList] => Node:" << m_currentMobility.imsi
                              << " lost ClusterHead " << value.imsi);

                //!< go to STANDALONE State
                m_currentMobility.clusterId = 0;
                m_currentMobility.degree = V2vClusterSap::STANDALONE;

                NS_LOG_DEBUG ("[UpdateNeighborList] => Go to STANDALONE state: " << m_currentMobility.imsi);
            }

            if(m_rStableList.find(key) != m_rStableList.end()){
                NS_LOG_DEBUG ("[UpdateNeighborList] => Node:" << m_currentMobility.imsi << " deletes CM from m_rStableList: " << value.imsi);
                m_rStableList.erase(key);
            }

            if(m_clusterList.find(key) != m_clusterList.end()){
                NS_LOG_DEBUG ("[UpdateNeighborList] => CH Node:" << m_currentMobility.imsi << " deletes CM from m_clusterList: " << value.imsi);
                m_clusterList.erase(key);
            }

            if( (m_rStableList.size() == 0) && (m_currentMobility.degree != V2vClusterSap::CH) ){
                NS_LOG_DEBUG ("[UpdateNeighborList] => To Become new CH: " << m_currentMobility.imsi);
                m_currentMobility.degree = V2vClusterSap::CH;
                m_currentMobility.clusterId = m_currentMobility.imsi;
                m_changesCounter ++;
            }
        }
        else{
            ++it;
        }
    }
    Simulator::Schedule (m_interval, &V2vControlClient::UpdateNeighborList, this);
}

void
V2vControlClient::ScheduleIncidentEvent (Time dt){
    NS_LOG_FUNCTION (this << dt);
    m_sendIncidentEvent = Simulator::Schedule (dt, &V2vControlClient::SendIncident, this);
}

void
V2vControlClient::SendIncident (void){

    V2vClusterSap::IncidentInfo incidentInfo;
    incidentInfo.clusterId = m_currentMobility.clusterId;
    incidentInfo.incidentType = V2vClusterSap::EMERGENCY_EVENT;

    V2vIncidentEventHeader incidentHeader;
    incidentHeader.SetIncidentInfo (incidentInfo);
    m_incidentTimestamp = incidentHeader.GetTs();

    Ptr<Packet> packet = Create<Packet>(0);
    packet->AddHeader(incidentHeader);

    if( (m_currentMobility.degree == V2vClusterSap::CH) || (m_currentMobility.degree == V2vClusterSap::STANDALONE)){

        //!< Broadcast Incident to Cluster Members directly
        m_socket->Send(packet);
        if (InetSocketAddress::IsMatchingType(m_peer)) {
            NS_LOG_UNCOND ("[Send] Broadcast Incident Message from " << m_currentMobility.imsi << "=> At time " << Simulator::Now ().GetSeconds ()
                    <<" sent " << packet->GetSize () << " bytes to "
                    << InetSocketAddress::ConvertFrom(m_peer).GetIpv4 ()
                    << " port " << InetSocketAddress::ConvertFrom (m_peer).GetPort ()
                    << " - Event Type is:" << ToString (incidentHeader.GetIncidentInfo ().incidentType));
        } else if (Inet6SocketAddress::IsMatchingType(m_peer)) {
            NS_LOG_UNCOND ("[Send] Broadcast Incident Message from " << m_currentMobility.imsi << "=> At time " << Simulator::Now ().GetSeconds ()
                    <<" sent " << packet->GetSize () << " bytes to "
                    << packet->GetSize () << " bytes to "
                    << Inet6SocketAddress::ConvertFrom(m_peer).GetIpv6 ()
                    << " port " << Inet6SocketAddress::ConvertFrom (m_peer).GetPort ()
                    << " - Event Type is:" << ToString (incidentHeader.GetIncidentInfo ().incidentType));
        }
    }
    else{

        //!< Send Incident event to Cluster Head firstly
        m_socketIncident->Send(packet);
        m_incidentCounter ++;
        if (InetSocketAddress::IsMatchingType(m_peerIncident)) {
            NS_LOG_UNCOND ("[Send] Incident Message => At time " << Simulator::Now ().GetSeconds ()
                    << "s node[IMSI] ["<< m_currentMobility.imsi <<"] sent "
                    << packet->GetSize () << " bytes to "
                    << InetSocketAddress::ConvertFrom(m_peerIncident).GetIpv4 ()
                    << " port " << InetSocketAddress::ConvertFrom (m_peerIncident).GetPort ()
                    << " - Event Type is:" << ToString (incidentInfo.incidentType));
        } else if (Inet6SocketAddress::IsMatchingType(m_peerIncident)) {
            NS_LOG_INFO ("[Send] Incident Message => At time " << Simulator::Now ().GetSeconds ()
                    << "s node[IMSI] ["<< m_currentMobility.imsi <<"] sent "
                    << packet->GetSize () << " bytes to "
                    << Inet6SocketAddress::ConvertFrom(m_peerIncident).GetIpv6 ()
                    << " port " << Inet6SocketAddress::ConvertFrom (m_peerIncident).GetPort ()
                    << " - Event Type is:" << ToString (incidentInfo.incidentType));
        }
    }

    //!< Schedule event generation in random time
    ScheduleIncidentEvent (Seconds (m_incidentWindow));

}

} // Namespace ns3

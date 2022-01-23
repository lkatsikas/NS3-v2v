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

#ifndef V2V_CONTROL_CLIENT_H_
#define V2V_CONTROL_CLIENT_H_

#include <map>
#include "ns3/ptr.h"
#include "ns3/double.h"
#include "ns3/address.h"
#include "ns3/event-id.h"
#include "ns3/application.h"
#include "ns3/traced-callback.h"
#include "ns3/v2v-mobility-model.h"
#include "ns3/v2v-cluster-sap.h"
#include "ns3/v2v-cluster-header.h"

namespace ns3 {

class Socket;
class Address;

class V2vControlClient: public Application {
public:

	enum NodeStatus{
        CLUSTER_INITIALIZATION = 0,
        CLUSTER_HEAD_ELECTION,
        CLUSTER_FORMATION,
        CLUSTER_UPDATE,
        CLUSTER_STATES
	};

	/**
	 * \brief Get the type ID.
	 * \return the object TypeId
	 */
    static TypeId GetTypeId (void);

	V2vControlClient();
    virtual ~V2vControlClient ();

	/**
	 * \return pointer to listening socket
	 */
    Ptr<Socket> GetListeningSocket (void) const;

	/**
	 * \brief Return a pointer to associated socket.
	 * \return pointer to associated socket
	 */
    Ptr<Socket> GetSocket (void) const;

protected:
    virtual void DoDispose (void);

private:
	/// inherited from Application base class.
    virtual void StartApplication (void);    // Called at time specified by Start
    virtual void StopApplication (void);     // Called at time specified by Stop

    void StartListeningLocal (void);	// Called from StartApplication()
    void StopListeningLocal (void);	// Called from StopApplication()

    /**
     * \brief Print sent/received packets statistics.
     */
    void PrintStatistics (std::ostream &os);


    //!< Receive locally
	/**
	 * \brief Handle a packet received by the application
	 * \param socket the receiving socket
	 */
    void HandleRead (Ptr<Socket> socket);
	/**
	 * \brief Handle an incoming connection
	 * \param socket the incoming connection socket
	 * \param from the address the connection is from
	 */
    void HandleAccept (Ptr<Socket> socket, const Address& from);
	/**
	 * \brief Handle an connection close
	 * \param socket the connected socket
	 */
    void HandlePeerClose (Ptr<Socket> socket);
	/**
	 * \brief Handle an connection error
	 * \param socket the connected socket
	 */
    void HandlePeerError (Ptr<Socket> socket);


    //!< Send locally
	/**
	 * \brief Schedule the next packet transmission
	 * \param dt time interval between packets.
	 */
    void ScheduleTransmit (Time dt);

	/**
	 * \brief Send a packet
	 */
    void Send (void);

	/**
	 * \brief Handle a Connection Succeed event
	 * \param socket the connected socket
	 */
    void ConnectionSucceeded (Ptr<Socket> socket);

	/**
	 * \brief Handle a Connection Failed event
	 * \param socket the not connected socket
	 */
    void ConnectionFailed (Ptr<Socket> socket);



    //!< Cluster Functionality
    /**
	 * \brief Check if the speed of the node is the lowest in the cluster
	 * \return boolean
	 */
    bool IsSlowestNode (void);

    /**
     * \brief Update status
     */
    void UpdateNeighbors (void);

	/**
	 * \brief Start the clusterHead election procedure
	 */
    void InitiateCluster (void);

	/**
	 * \brief Check if a node is suitable for CH election
	 * \return integer that represents current suitability value
	 */
    double SuitabilityCheck (void);

    /**
     * @brief IsStable checks if the relative velocity of two neighbors
     * is under the threshold
     * @param velocity, the velocity of the neighbor node
     * @return true if stable, false otherwise
     */
    bool IsStable (Vector velocity);

	/**
	 * \brief Start the clusterHead election procedure
	 */
    void FormCluster (void);

	/**
	 * \brief Send Initiate Cluster message
	 */
	void ScheduleInitiateCluster (Time dt);

	/**
	 * \brief Send Initiate Cluster message
	 */
    void SendInitiateCluster (void);

	/**
	 * \brief Report the status of the node
	 */
    void StatusReport (void);

    /**
     * @brief Update Neighbor's lists according to latest Timestamps
     */
    void UpdateNeighborList (void);

    /**
     * \brief return the id of the suitable CH
     */
    uint64_t MergeCheck (void);

    /**
     * @brief Acquire current mobility info
     */
    void AcquireMobilityInfo (void);



    //!< Incident
    /**
     * @brief CreateIncidentSocket
     */
    void CreateIncidentSocket (Address from);

    /**
     * @brief RemoveIncidentSocket
     */
    void RemoveIncidentSocket (void);

    /**
     * @brief ScheduleIncidentEvent
     */
    void ScheduleIncidentEvent (Time dt);

    /**
     * @brief SendIncident
     */
    void SendIncident (void);

    /**
     * \brief Handle a Connection Succeed event
     * \param socket the connected socket
     */
    void ConnectionCHSucceeded (Ptr<Socket> socket);

    /**
     * \brief Handle a Connection Failed event
     * \param socket the not connected socket
     */
    void ConnectionCHFailed (Ptr<Socket> socket);


    uint32_t m_formationCounter;            //!< Count the cluster formation messages
    uint32_t m_changesCounter;              //!< Count the changes of the state of the vehicle
    double m_incidentWindow;                //!< Time Window for incident event generation

    /* Incident Report */
    Time m_incidentTimestamp;               //!< Timestamp of last incident report
    TypeId m_tidIncident;   				//!< Type of the socket used
    Address m_peerIncident;         		//!< Peer address
    double m_overalDelay;                   //!< The aggregated delay of the incident messages
    uint32_t m_incidentCounter;             //!< Counter for sent incident packets
    Ptr<Socket> m_socketIncident;           //!< Socket with Cluster Head to send incidents
    EventId m_sendIncidentEvent;            //!< Event id of scheduled incident

	/* Receive Socket */
	TypeId m_tidListening;          		//!< Protocol TypeId
	Address m_peerListening;       	 		//!< Local address to bind to
	Ptr<Socket> m_socketListening;      	//!< Listening socket

	/* Send Socket */
	TypeId m_tid;          					//!< Type of the socket used
	Address m_peer;        	 				//!< Peer address
	EventId m_sendEvent;    				//!< Event id of pending "send packet" event
	Ptr<Socket> m_socket;       			//!< Associated socket

    Time m_interval; 						//!< Packet inter-send time
    double m_timeWindow;                    //!< Time Window for cluster formation
	uint32_t m_pktSize;      				//!< Size of packets
    uint32_t m_sentCounter; 				//!< Counter for sent packets

    TracedCallback<Ptr<const Packet> > m_txTrace;
	TracedCallback<Ptr<const Packet>, const Address &> m_rxTrace;

    uint32_t m_maxUes;                      //!< maximun number of ues
    double m_minimumTdmaSlot;               //!< the minimum tdma slot
    double m_clusterTimeMetric;             //!< the timeslot for the node to schedule transmission

	/// Node information for cluster formation
    EventId m_chElectionEvent;                          //!< Event id of pending "CH Request" event
    V2vClusterSap::NeighborInfo m_currentMobility;
    Ptr<V2vMobilityModel> m_mobilityModel;
    enum NodeStatus m_status;                           //!< Node Degree
    std::map<uint64_t, V2vClusterSap::NeighborInfo> m_clusterList;     //!< Cluster List
    std::map<uint64_t, V2vClusterSap::NeighborInfo> m_rStableList;     //!< rStable Neighbors
    std::map<uint64_t, V2vClusterSap::NeighborInfo> m_2rStableList;    //!< 2rStable Neighbors

    //!< Incident Info
    V2vClusterSap::IncidentInfo m_incidentInfo;

};

} // namespace ns3

#endif /* V2V_CONTROL_CLIENT_H_ */

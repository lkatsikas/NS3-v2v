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

#ifndef V2V_CONTROL_CLIENT_HELPER_H
#define V2V_CONTROL_CLIENT_HELPER_H

#include <stdint.h>
#include "ns3/object-factory.h"
#include "ns3/address.h"
#include "ns3/attribute.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "ns3/v2v-control-client.h"

namespace ns3 {

/**
 * \brief A helper to make it easier to instantiate an ns3::V2vControlClient
 * on a set of nodes.
 */
class V2vControlClientHelper {
public:

    /**
     * @brief Create an V2vControlClientHelper to make it easier to work with V2vControlClient
     * @param protocolLocal the name of the protocol to use to send traffic
     * @param addressLocal the address of the node(s) to send traffic
     * @param protocolListeningLocal the name of the protocol to use to receive traffic
     * @param addressListeningLocal the address to receive traffic
     * @param mobilityModel a pointer to the mobility model of the node
     * @param timeFactor
     */
    V2vControlClientHelper(std::string protocolLocal, Address addressLocal,
        std::string protocolListeningLocal, Address addressListeningLocal,
        Ptr<V2vMobilityModel> mobilityModel, double timeFactor, uint32_t maxUes, double minimumTdmaSlot, double clusterTimeMetric);

	/**
	 * Helper function used to set the underlying application attributes.
	 *
	 * \param name the name of the application attribute to set
	 * \param value the value of the application attribute to set
	 */
	void SetAttribute(std::string name, const AttributeValue &value);

	/**
	 * Install an ns3::V2vControlClient on each node of the input container
	 * configured with all the attributes set with SetAttribute.
	 *
	 * \param c NodeContainer of the set of nodes on which an V2vControlClient
	 * will be installed.
	 * \returns Container of Ptr to the applications installed.
	 */
	ApplicationContainer Install(NodeContainer c) const;

	/**
	 * Install an ns3::V2vControlClient on the node configured with all the
	 * attributes set with SetAttribute.
	 *
	 * \param node The node on which an V2vControlClient will be installed.
	 * \returns Container of Ptr to the applications installed.
	 */
	ApplicationContainer Install(Ptr<Node> node) const;

	/**
	 * Install an ns3::V2vControlClient on the node configured with all the
	 * attributes set with SetAttribute.
	 *
	 * \param nodeName The node on which an V2vControlClient will be installed.
	 * \returns Container of Ptr to the applications installed.
	 */
	ApplicationContainer Install(std::string nodeName) const;

private:
	/**
	 * Install an ns3::V2vControlClient on the node configured with all the
	 * attributes set with SetAttribute.
	 *
	 * \param node The node on which an V2vControlClient will be installed.
	 * \returns Ptr to the application installed.
	 */
	Ptr<Application> InstallPriv(Ptr<Node> node) const;

    ObjectFactory m_factory;
};

} // namespace ns3

#endif /* V2V_CONTROL_CLIENT_HELPER_H */

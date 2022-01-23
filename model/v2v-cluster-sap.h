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

#ifndef V2V_CLUSTER_SAP_H
#define V2V_CLUSTER_SAP_H

#include <list>
#include "ns3/nstime.h"
#include "ns3/vector.h"

namespace ns3 {

class V2vClusterSap {

public:
    virtual ~V2vClusterSap ();

    enum NodeDegree{
        STANDALONE = 0,
        CH,
        CM,
        DEGREE_STATES
    };

    struct NeighborInfo{
        Time ts;
        uint64_t imsi;
        uint64_t clusterId;

        Vector position;
        Vector velocity;
        Vector direction;
        NodeDegree degree;
    };

    enum IncidentType{
        EMERGENCY_EVENT = 0,
        NOTIFICATION_EVENT,
        INCIDENT_STATES
    };

    struct IncidentInfo{
        uint64_t clusterId;
        IncidentType incidentType;
    };

private:

};

} // namespace ns3

#endif // V2V_CLUSTER_SAP_H


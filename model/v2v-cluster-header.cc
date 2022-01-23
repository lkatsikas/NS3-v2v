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

#include "ns3/simulator.h"
#include "v2v-cluster-header.h"

NS_LOG_COMPONENT_DEFINE ("V2vClusterHeader");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(V2vClusterInfoHeader);

V2vClusterInfoHeader::V2vClusterInfoHeader() :
   m_seq(0){
   NS_LOG_FUNCTION (this);
}

V2vClusterInfoHeader::~V2vClusterInfoHeader(){
   NS_LOG_FUNCTION (this);
}

void
V2vClusterInfoHeader::SetSeq(uint64_t seq) {
   NS_LOG_FUNCTION (this << seq);
   m_seq = seq;
}

uint64_t
V2vClusterInfoHeader::GetSeq(void) const {
   NS_LOG_FUNCTION (this);
   return m_seq;
}

void
V2vClusterInfoHeader::SetMobilityInfo(V2vClusterSap::NeighborInfo mobilityInfo){
   NS_LOG_FUNCTION (this << mobilityInfo.imsi);
   m_mobilityInfo = mobilityInfo;
}

V2vClusterSap::NeighborInfo
V2vClusterInfoHeader::GetMobilityInfo(void) const {
   NS_LOG_FUNCTION (this);
   return m_mobilityInfo;
}

TypeId
V2vClusterInfoHeader::GetTypeId(void) {
   static TypeId tid = TypeId("ns3::V2vClusterInfoHeader").SetParent<Header>().AddConstructor<V2vClusterInfoHeader>();
   return tid;
}

TypeId
V2vClusterInfoHeader::GetInstanceTypeId(void) const {
   return GetTypeId();
}

void
V2vClusterInfoHeader::Print(std::ostream &os) const {
   NS_LOG_FUNCTION (this << &os);
   os << "(seq=" << m_seq
           << "IMSI=" << m_mobilityInfo.imsi
           << "ClusterId=" << m_mobilityInfo.clusterId
           << "Degree=" << m_mobilityInfo.degree
           << "Position=" << m_mobilityInfo.position
           << "Velocity=" << m_mobilityInfo.velocity
           << "Direction=" << m_mobilityInfo.direction <<")";
}

uint32_t
V2vClusterInfoHeader::GetSerializedSize(void) const {
   NS_LOG_FUNCTION (this);
   return sizeof(uint64_t) + sizeof(V2vClusterSap::NeighborInfo);
}

void
V2vClusterInfoHeader::Serialize(Buffer::Iterator start) const {
   NS_LOG_FUNCTION (this << &start);

   Buffer::Iterator i = start;
   i.WriteHtonU64(m_seq);

   // Write mobility structure
   unsigned char temp[sizeof(V2vClusterSap::NeighborInfo)];
   memcpy( temp, &m_mobilityInfo, sizeof(V2vClusterSap::NeighborInfo) );
   i.Write(temp, sizeof(V2vClusterSap::NeighborInfo));

}

uint32_t
V2vClusterInfoHeader::Deserialize(Buffer::Iterator start) {
   NS_LOG_INFO (this << &start);

   Buffer::Iterator i = start;
   m_seq = i.ReadNtohU64();

   unsigned char temp[sizeof(V2vClusterSap::NeighborInfo)];
   i.Read(temp, sizeof(V2vClusterSap::NeighborInfo));
   memcpy(&m_mobilityInfo, &temp, sizeof(V2vClusterSap::NeighborInfo));

   return GetSerializedSize();
}


/////////////////////////////////////////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED(V2vInitiateClusterHeader);


V2vInitiateClusterHeader::V2vInitiateClusterHeader() :
        m_clusterId(0),
        m_ts(Simulator::Now().GetTimeStep()),
        m_seq(0){
    NS_LOG_FUNCTION (this);
}

V2vInitiateClusterHeader::~V2vInitiateClusterHeader(){
    NS_LOG_FUNCTION (this);
}

Time
V2vInitiateClusterHeader::GetTs(void) const {
    NS_LOG_FUNCTION (this);
    return TimeStep(m_ts);
}

void
V2vInitiateClusterHeader::SetSeq(uint64_t seq) {
    NS_LOG_FUNCTION (this << seq);
    m_seq = seq;
}
uint64_t
V2vInitiateClusterHeader::GetSeq(void) const {
    NS_LOG_FUNCTION (this);
    return m_seq;
}

void
V2vInitiateClusterHeader::SetClusterId(uint64_t clusterId){
    NS_LOG_FUNCTION (this << clusterId);
    m_clusterId = clusterId;
}

uint64_t
V2vInitiateClusterHeader::GetClusterId(void) const {
    NS_LOG_FUNCTION (this);
    return m_clusterId;
}

TypeId
V2vInitiateClusterHeader::GetTypeId(void) {
    static TypeId tid =
            TypeId("ns3::V2vInitiateClusterHeader").SetParent<Header>().AddConstructor<V2vInitiateClusterHeader>();
    return tid;
}

TypeId
V2vInitiateClusterHeader::GetInstanceTypeId(void) const {
    return GetTypeId();
}

void
V2vInitiateClusterHeader::Print(std::ostream &os) const {
    NS_LOG_FUNCTION (this << &os);
    os << "(time=" << TimeStep(m_ts).GetSeconds() << " ClusterId=" << m_clusterId << " Seq=" << m_seq <<")";
}

uint32_t
V2vInitiateClusterHeader::GetSerializedSize(void) const {
    NS_LOG_FUNCTION (this);
    return sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint64_t);
}

void
V2vInitiateClusterHeader::Serialize(Buffer::Iterator start) const {
    NS_LOG_FUNCTION (this << &start);

    Buffer::Iterator i = start;
    i.WriteHtonU64(m_clusterId);
    i.WriteHtonU64(m_ts);
    i.WriteHtonU64(m_seq);
}

uint32_t
V2vInitiateClusterHeader::Deserialize(Buffer::Iterator start) {
    NS_LOG_INFO (this << &start);

    Buffer::Iterator i = start;
    m_clusterId = i.ReadNtohU64 ();
    m_ts = i.ReadNtohU64 ();
    m_seq = i.ReadNtohU64 ();

    return GetSerializedSize();
}


/////////////////////////////////////////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED(V2vFormClusterHeader);

V2vFormClusterHeader::V2vFormClusterHeader() :
        m_seq(0){
    NS_LOG_FUNCTION (this);
}

V2vFormClusterHeader::~V2vFormClusterHeader(){
    NS_LOG_FUNCTION (this);
}

void
V2vFormClusterHeader::SetSeq(uint64_t seq) {
    NS_LOG_FUNCTION (this << seq);
    m_seq = seq;
}
uint64_t
V2vFormClusterHeader::GetSeq(void) const {
    NS_LOG_FUNCTION (this);
    return m_seq;
}

void
V2vFormClusterHeader::SetMobilityInfo(V2vClusterSap::NeighborInfo mobilityInfo){
    NS_LOG_FUNCTION (this << mobilityInfo.imsi);
    m_mobilityInfo = mobilityInfo;
}

V2vClusterSap::NeighborInfo
V2vFormClusterHeader::GetMobilityInfo(void) const {
    NS_LOG_FUNCTION (this);
    return m_mobilityInfo;
}

TypeId
V2vFormClusterHeader::GetTypeId(void) {
    static TypeId tid =
            TypeId("ns3::V2vFormClusterHeader").SetParent<Header>().AddConstructor<V2vFormClusterHeader>();
    return tid;
}

TypeId
V2vFormClusterHeader::GetInstanceTypeId(void) const {
    return GetTypeId();
}

void
V2vFormClusterHeader::Print(std::ostream &os) const {
    NS_LOG_FUNCTION (this << &os);
    os << "(seq=" << m_seq
            << "IMSI=" << m_mobilityInfo.imsi
            << "ClusterId=" << m_mobilityInfo.clusterId
            << "Degree=" << m_mobilityInfo.degree
            << "Position=" << m_mobilityInfo.position
            << "Velocity=" << m_mobilityInfo.velocity
            << "Direction=" << m_mobilityInfo.direction <<")";
}

uint32_t
V2vFormClusterHeader::GetSerializedSize(void) const {
    NS_LOG_FUNCTION (this);
    return sizeof(uint64_t) + sizeof(V2vClusterSap::NeighborInfo);
}

void
V2vFormClusterHeader::Serialize(Buffer::Iterator start) const {
    NS_LOG_FUNCTION (this << &start);

    Buffer::Iterator i = start;
    i.WriteHtonU64(m_seq);

    // Write mobility structure
    unsigned char temp[sizeof(V2vClusterSap::NeighborInfo)];
    memcpy( temp, &m_mobilityInfo, sizeof(V2vClusterSap::NeighborInfo));
    i.Write(temp, sizeof(V2vClusterSap::NeighborInfo));

}

uint32_t
V2vFormClusterHeader::Deserialize(Buffer::Iterator start) {
    NS_LOG_INFO (this << &start);

    Buffer::Iterator i = start;
    m_seq = i.ReadNtohU64();

    unsigned char temp[sizeof(V2vClusterSap::NeighborInfo)];
    i.Read(temp, sizeof(V2vClusterSap::NeighborInfo));
    memcpy(&m_mobilityInfo, &temp, sizeof(V2vClusterSap::NeighborInfo) );

    return GetSerializedSize();
}


/////////////////////////////////////////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED(V2vIncidentEventHeader);

V2vIncidentEventHeader::V2vIncidentEventHeader():
    m_ts(Simulator::Now().GetTimeStep()){
    NS_LOG_FUNCTION (this);
}

V2vIncidentEventHeader::~V2vIncidentEventHeader(){
    NS_LOG_FUNCTION (this);
}

Time
V2vIncidentEventHeader::GetTs(void) const {
    NS_LOG_FUNCTION (this);
    return TimeStep(m_ts);
}

void
V2vIncidentEventHeader::SetIncidentInfo(V2vClusterSap::IncidentInfo incidentInfo) {
    NS_LOG_FUNCTION (this << incidentInfo.clusterId);
    m_incidentInfo = incidentInfo;
}

V2vClusterSap::IncidentInfo
V2vIncidentEventHeader::GetIncidentInfo(void) const {
    NS_LOG_FUNCTION (this);
    return m_incidentInfo;
}

TypeId
V2vIncidentEventHeader::GetTypeId(void) {
    static TypeId tid =
            TypeId("ns3::V2vIncidentEventHeader").SetParent<Header>().AddConstructor<V2vIncidentEventHeader>();
    return tid;
}

TypeId
V2vIncidentEventHeader::GetInstanceTypeId(void) const {
    return GetTypeId();
}

void
V2vIncidentEventHeader::Print(std::ostream &os) const {
    NS_LOG_FUNCTION (this << &os);
    os << "(time=" << TimeStep(m_ts).GetSeconds() << " ClusterId=" << m_incidentInfo.clusterId
       << " IncidentType =" << m_incidentInfo.incidentType << ")";
}

uint32_t
V2vIncidentEventHeader::GetSerializedSize(void) const {
    NS_LOG_FUNCTION (this);
    return sizeof(uint64_t) + sizeof(V2vClusterSap::IncidentInfo);
}

void
V2vIncidentEventHeader::Serialize(Buffer::Iterator start) const {
    NS_LOG_FUNCTION (this << &start);

    Buffer::Iterator i = start;
    i.WriteHtonU64(m_ts);

    // Write IncidentInfo structure
    unsigned char temp[sizeof(V2vClusterSap::IncidentInfo)];
    memcpy( temp, &m_incidentInfo, sizeof(V2vClusterSap::IncidentInfo) );
    i.Write(temp, sizeof(V2vClusterSap::IncidentInfo));
}

uint32_t
V2vIncidentEventHeader::Deserialize(Buffer::Iterator start) {
    NS_LOG_INFO (this << &start);

    Buffer::Iterator i = start;
    m_ts = i.ReadNtohU64 ();

    unsigned char temp[sizeof(V2vClusterSap::IncidentInfo)];
    i.Read(temp, sizeof(V2vClusterSap::IncidentInfo));
    memcpy(&m_incidentInfo, &temp, sizeof(V2vClusterSap::IncidentInfo) );

    return GetSerializedSize();
}

} // namespace ns3

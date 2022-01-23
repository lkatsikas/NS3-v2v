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

#include <fstream>
#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/abort.h"
#include "ns3/config.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/simulator.h"
#include "ns3/simple-channel.h"
#include "ns3/simple-net-device.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/inet-socket-address.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/v2v-cluster-header.h"
#include "ns3/v2v-control-client.h"

using namespace ns3;


/*--------------------------- V2vControlClient Testing ---------------------------*/
class V2vControlClientTestCase: public TestCase {
public:
    V2vControlClientTestCase();
    virtual ~V2vControlClientTestCase();

private:
	virtual void DoRun(void);

};

V2vControlClientTestCase::V2vControlClientTestCase() :
        TestCase("Verify the correct transmission of the packets."){
}

V2vControlClientTestCase::~V2vControlClientTestCase() {
}

void V2vControlClientTestCase::DoRun(void) {

	Simulator::Run();
	Simulator::Destroy();

}


/*--------------------------- V2vClusterInfoHeader Testing ---------------------------*/
class V2vClusterInfoHeaderTestCase: public TestCase {
public:
    V2vClusterInfoHeaderTestCase();
    virtual ~V2vClusterInfoHeaderTestCase();

private:
	virtual void DoRun(void);

};

V2vClusterInfoHeaderTestCase::V2vClusterInfoHeaderTestCase() :
        TestCase("Check V2vClusterInfoHeader class serialization-deserialization"){
}

V2vClusterInfoHeaderTestCase::~V2vClusterInfoHeaderTestCase() {
}

void V2vClusterInfoHeaderTestCase::DoRun(void) {

    Simulator::Run();
    Simulator::Destroy();
}
/*--------------------------------------------------------------------------*/

/*--------------------------- V2vInitiateClusterHeader Testing ---------------------------*/
class V2vInitiateClusterHeaderTestCase: public TestCase {
public:
    V2vInitiateClusterHeaderTestCase();
    virtual ~V2vInitiateClusterHeaderTestCase();

private:
    virtual void DoRun(void);

};

V2vInitiateClusterHeaderTestCase::V2vInitiateClusterHeaderTestCase() :
        TestCase("Check V2vInitiateClusterHeader class serialization-deserialization"){
}

V2vInitiateClusterHeaderTestCase::~V2vInitiateClusterHeaderTestCase() {
}

void V2vInitiateClusterHeaderTestCase::DoRun(void) {

    Simulator::Run();
    Simulator::Destroy();
}
/*--------------------------------------------------------------------------*/

/*--------------------------- V2vFormClusterHeader Testing ---------------------------*/
class V2vFormClusterHeaderTestCase: public TestCase {
public:
    V2vFormClusterHeaderTestCase();
    virtual ~V2vFormClusterHeaderTestCase();

private:
    virtual void DoRun(void);

};

V2vFormClusterHeaderTestCase::V2vFormClusterHeaderTestCase() :
        TestCase("Check V2vFormClusterHeader class serialization-deserialization"){
}

V2vFormClusterHeaderTestCase::~V2vFormClusterHeaderTestCase() {
}

void V2vFormClusterHeaderTestCase::DoRun(void) {

    Simulator::Run();
    Simulator::Destroy();
}
/*--------------------------------------------------------------------------*/

/*--------------------------- V2vIncidentEventHeader Testing ---------------------------*/
class V2vIncidentEventHeaderTestCase: public TestCase {
public:
    V2vIncidentEventHeaderTestCase();
    virtual ~V2vIncidentEventHeaderTestCase();

private:
    virtual void DoRun(void);

};

V2vIncidentEventHeaderTestCase::V2vIncidentEventHeaderTestCase() :
        TestCase("Check V2vIncidentEventHeader class serialization-deserialization"){
}

V2vIncidentEventHeaderTestCase::~V2vIncidentEventHeaderTestCase() {
}

void V2vIncidentEventHeaderTestCase::DoRun(void) {

    Simulator::Run();
    Simulator::Destroy();
}
/*--------------------------------------------------------------------------*/

/*--------------------------- End-to-end Testing ---------------------------*/
class V2vUdpEndToEndTestCase: public TestCase {
public:
	V2vUdpEndToEndTestCase();
	virtual ~V2vUdpEndToEndTestCase();

private:
	virtual void DoRun(void);

};

V2vUdpEndToEndTestCase::V2vUdpEndToEndTestCase() :
        TestCase("Test an end-to-end clustering scenario."){
}

V2vUdpEndToEndTestCase::~V2vUdpEndToEndTestCase() {
}

void V2vUdpEndToEndTestCase::DoRun(void) {
	// Todo:
}
/*--------------------------------------------------------------------------*/


/*------------------------ TestSuite Initialization ------------------------*/
class V2vTestSuite: public TestSuite {
public:
	V2vTestSuite();
};

V2vTestSuite::V2vTestSuite() : TestSuite("v2v", UNIT) {
    AddTestCase(new V2vControlClientTestCase, TestCase::QUICK);
    AddTestCase(new V2vClusterInfoHeaderTestCase, TestCase::QUICK);
    AddTestCase(new V2vInitiateClusterHeaderTestCase, TestCase::QUICK);
    AddTestCase(new V2vFormClusterHeaderTestCase, TestCase::QUICK);
    AddTestCase(new V2vIncidentEventHeaderTestCase, TestCase::QUICK);
	AddTestCase(new V2vUdpEndToEndTestCase, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static V2vTestSuite v2vTestSuite;


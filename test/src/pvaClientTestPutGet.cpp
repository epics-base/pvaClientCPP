/*pvaClientTestPutGet.cpp */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvData is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
/**
 * @author mrk
 */

/* Author: Marty Kraimer */

#include <iostream>

#include <pv/pvaClient.h>
#include <epicsUnitTest.h>
#include <testMain.h>

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvaClient;


static void example(PvaClientPtr const &pvaClient)
{
    cout << "\nstarting channelPutGet example\n";
    try {
cout << "calling createChannel\n";
        PvaClientChannelPtr pvaChannel = pvaClient->createChannel("examplePowerSupply");
cout << "calling connect\n";
        pvaChannel->connect(2.0);
        testOk(true==true,"connected");
cout << "calling createPutGet\n";
        PvaClientPutGetPtr putGet = pvaChannel->createPutGet(
            "putField(power.value,voltage.value)getField()");
cout << "calling getPutData\n";
        PvaClientPutDataPtr putData = putGet->getPutData();
        testOk(true==true,"put connected");
        PVStructurePtr pvStructure = putData->getPVStructure();
        PVDoublePtr power = pvStructure->getSubField<PVDouble>("power.value");
        PVDoublePtr voltage = pvStructure->getSubField<PVDouble>("voltage.value");
        power->put(5.0);
        voltage->put(5.0);
        putGet->putGet();
        PvaClientGetDataPtr getData = putGet->getGetData();
        pvStructure = getData->getPVStructure();
        BitSetPtr bitSet = getData->getBitSet();
        cout << "changed " << getData->showChanged(cout) << endl;
        cout << "bitSet " << *bitSet << endl;
        power->put(6.0);
        putGet->putGet();
        pvStructure = getData->getPVStructure();
        bitSet = getData->getBitSet();
        cout << "changed " << getData->showChanged(cout) << endl;
    } catch (std::runtime_error e) {
        cout << "exception " << e.what() << endl;
    }
}


MAIN(pvaClientTestPutGet)
{
    cout << "\nstarting pvaClientTestPutGet\n";
    testPlan(2);
    PvaClientPtr pvaClient = PvaClient::create();
    example(pvaClient);
    cout << "done\n";
    return 0;
}

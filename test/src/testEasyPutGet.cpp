/*exampleEasyPutGet.cpp */
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

#include <pv/easyPVA.h>
#include <epicsUnitTest.h>
#include <testMain.h>

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::easyPVA;


static void example(EasyPVAPtr const &easyPVA)
{
    cout << "\nstarting channelPutGet example\n";
    try {
        EasyChannelPtr easyChannel = easyPVA->createChannel("examplePowerSupply");
        easyChannel->connect(2.0);
        testOk(true==true,"connected");
        EasyPutGetPtr putGet = easyChannel->createPutGet(
            "putField(power.value,voltage.value)getField()");
        EasyPutDataPtr putData = putGet->getPutData();
        testOk(true==true,"put connected");
        PVStructurePtr pvStructure = putData->getPVStructure();
        PVDoublePtr power = pvStructure->getSubField<PVDouble>("power.value");
        PVDoublePtr voltage = pvStructure->getSubField<PVDouble>("voltage.value");
        power->put(5.0);
        voltage->put(5.0);
        putGet->putGet();
        EasyGetDataPtr getData = putGet->getGetData();
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


MAIN(testEasyPutGet)
{
    cout << "\nstarting testEasyPutGet\n";
    testPlan(2);
    EasyPVAPtr easyPVA = EasyPVA::create();
    example(easyPVA);
    cout << "done\n";
    return 0;
}

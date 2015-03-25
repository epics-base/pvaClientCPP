/*helloWorldPutGet.cpp */
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

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::easyPVA;


static void example(EasyPVAPtr const &easyPVA)
{
    cout << "helloWorldPutGet\n";
    try {
        EasyChannelPtr channel = easyPVA->channel("exampleHello");
        EasyPutGetPtr putGet = channel->createPutGet();
        putGet->connect();
        EasyPutDataPtr putData = putGet->getPutData();
        PVStructurePtr arg = putData->getPVStructure();
        PVStringPtr pvValue = arg->getSubField<PVString>("argument.value");
        pvValue->put("World");
        putGet->putGet();
        EasyGetDataPtr getData = putGet->getGetData();
        cout << getData->getPVStructure() << endl;
    } catch (std::runtime_error e) {
        cout << "exception " << e.what() << endl;
    }
}


int main(int argc,char *argv[])
{
    EasyPVAPtr easyPVA = EasyPVA::create();
    example(easyPVA);
    return 0;
}

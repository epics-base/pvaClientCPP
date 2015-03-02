/*exampleEasyGet.cpp */
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

static EasyPVAPtr easyPVA;

static void exampleDouble()
{
    cout << "example double scalar\n";
    double value;
    try {
        cout << "short way\n";
        value =  easyPVA->createChannel("exampleDouble")->createGet()->getDouble();
        cout << "as double " << value << endl;
    } catch (std::runtime_error e) {
        cout << "exception " << e.what() << endl;
    }
    try {
        cout << "long way\n";
        EasyChannelPtr easyChannel = easyPVA->createChannel("exampleDouble");
        easyChannel->connect(2.0);
        EasyGetPtr easyGet = easyChannel->createGet();
        value = easyGet->getDouble();
        cout << "as double " << value << endl;
    } catch (std::runtime_error e) {
        cout << "exception " << e.what() << endl;
    }
}

static void exampleDoubleArray()
{
    cout << "example double array\n";
    shared_vector<double> value;
    try {
        cout << "short way\n";
        value =  easyPVA->createChannel("exampleDoubleArray")->createGet()->getDoubleArray();
        cout << "as doubleArray " << value << endl;
    } catch (std::runtime_error e) {
        cout << "exception " << e.what() << endl;
    }
    try {
        cout << "long way\n";
        EasyChannelPtr easyChannel = easyPVA->createChannel("exampleDoubleArray");
        easyChannel->connect(2.0);
        EasyGetPtr easyGet = easyChannel->createGet();
        value = easyGet->getDoubleArray();
        cout << "as doubleArray " << value << endl;
    } catch (std::runtime_error e) {
        cout << "exception " << e.what() << endl;
    }
}

static void examplePowerSupply()
{
    cout << "example powerSupply\n";
    PVStructurePtr pvStructure;
    try {
        cout << "short way\n";
        pvStructure =  easyPVA->createChannel("examplePowerSupply")->createGet("field()")->getPVStructure();
        cout << pvStructure << endl;
    } catch (std::runtime_error e) {
        cout << "exception " << e.what() << endl;
    }
     
}

int main(int argc,char *argv[])
{
    easyPVA = EasyPVA::create();
    exampleDouble();
    exampleDoubleArray();
    examplePowerSupply();
    cout << "done\n";
    easyPVA->destroy();
    return 0;
}

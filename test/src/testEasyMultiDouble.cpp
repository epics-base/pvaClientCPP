/*testEasyMultiDouble.cpp */
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

#include <pv/easyMultiDouble.h>
#include <epicsUnitTest.h>
#include <testMain.h>

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::easyPVA;


static void testGood(EasyPVAPtr const &easyPVA)
{
    bool isOk = true;
    cout << "\nstarting testGood\n";
    try {
        EasyPVAPtr easyPVA(EasyPVA::create());
        size_t num = 5;
        shared_vector<string> channelNames(num);
        channelNames[0] = "exampleDouble01";
        channelNames[1] = "exampleDouble02";
        channelNames[2] = "exampleDouble03";
        channelNames[3] = "exampleDouble04";
        channelNames[4] = "exampleDouble05";
        PVStringArrayPtr pvNames =
            getPVDataCreate()->createPVScalarArray<PVStringArray>();
        pvNames->replace(freeze(channelNames));
        EasyMultiDoublePtr multiDouble(EasyMultiDouble::create(easyPVA,pvNames));
        shared_vector<double> data = multiDouble->get();
        cout << "initial " << data << endl;
        for(size_t i=0; i<num; ++i) data[i] = data[i] + 1.1;
        multiDouble->put(data);
        data =  multiDouble->get();
        cout << "final " << data << endl;
    } catch (std::runtime_error e) {
        cout << "exception " << e.what() << endl;
        isOk = false;
    }
    testOk(isOk==true,"all channels double");
}

static void testGoodMixed(EasyPVAPtr const &easyPVA)
{
    bool isOk = true;
    cout << "\nstarting testGoodMixed\n";
    try {
        EasyPVAPtr easyPVA(EasyPVA::create());
        size_t num = 5;
        shared_vector<string> channelNames(num);
        channelNames[0] = "exampleByte";
        channelNames[1] = "exampleShort";
        channelNames[2] = "exampleInt";
        channelNames[3] = "exampleFloat";
        channelNames[4] = "exampleDouble";
        PVStringArrayPtr pvNames =
            getPVDataCreate()->createPVScalarArray<PVStringArray>();
        pvNames->replace(freeze(channelNames));
        EasyMultiDoublePtr multiDouble(EasyMultiDouble::create(easyPVA,pvNames));
        shared_vector<double> data = multiDouble->get();
        cout << "initial " << data << endl;
        for(size_t i=0; i<num; ++i) data[i] = data[i] + 1.1;
        multiDouble->put(data);
        data =  multiDouble->get();
        cout << "final " << data << endl;
    } catch (std::runtime_error e) {
        cout << "exception " << e.what() << endl;
        isOk = false;
    }
    testOk(isOk==true,"channels mixed type");
}

static void testChannelNotExist(EasyPVAPtr const &easyPVA)
{
    bool isOk = true;
    cout << "\nstarting testChannelNotExist\n";
    try {
        EasyPVAPtr easyPVA(EasyPVA::create());
        size_t num = 5;
        shared_vector<string> channelNames(num);
        channelNames[0] = "exampleDouble01";
        channelNames[1] = "exampleDouble02";
        channelNames[2] = "exampleDouble03";
        channelNames[3] = "NoneExistChannel";
        channelNames[4] = "exampleDouble05";
        PVStringArrayPtr pvNames =
            getPVDataCreate()->createPVScalarArray<PVStringArray>();
        pvNames->replace(freeze(channelNames));
        EasyMultiDoublePtr multiDouble(EasyMultiDouble::create(easyPVA,pvNames));
        shared_vector<double> data = multiDouble->get();
        cout << "initial " << data << endl;
        for(size_t i=0; i<num; ++i) data[i] = data[i] + 1.1;
        multiDouble->put(data);
        data =  multiDouble->get();
        cout << "final " << data << endl;
    } catch (std::runtime_error e) {
        cout << "exception " << e.what() << endl;
        isOk = false;
    }
    testOk(isOk==false,"channel not exist");
}

static void testNonNumeric(EasyPVAPtr const &easyPVA)
{
    bool isOk = true;
    cout << "\nstarting testNonNumeric\n";
    try {
        EasyPVAPtr easyPVA(EasyPVA::create());
        size_t num = 5;
        shared_vector<string> channelNames(num);
        channelNames[0] = "exampleDouble01";
        channelNames[1] = "exampleDouble02";
        channelNames[2] = "exampleDouble03";
        channelNames[3] = "exampleDouble04";
        channelNames[4] = "exampleDouble05Array";
        PVStringArrayPtr pvNames =
            getPVDataCreate()->createPVScalarArray<PVStringArray>();
        pvNames->replace(freeze(channelNames));
        EasyMultiDoublePtr multiDouble(EasyMultiDouble::create(easyPVA,pvNames));
        shared_vector<double> data = multiDouble->get();
        cout << "initial " << data << endl;
        for(size_t i=0; i<num; ++i) data[i] = data[i] + 1.1;
        multiDouble->put(data);
        data =  multiDouble->get();
        cout << "final " << data << endl;
    } catch (std::runtime_error e) {
        cout << "exception " << e.what() << endl;
        isOk = false;
    }
    testOk(isOk==false,"channel not numeric");
}

MAIN(testEasyMultiDouble)
{
    cout << "\nstarting testEasyMultiDouble\n";
    testPlan(4);
    EasyPVAPtr easyPVA = EasyPVA::create();
    testGood(easyPVA);
    testGoodMixed(easyPVA);
    testChannelNotExist(easyPVA);
    testNonNumeric(easyPVA);
    cout << "done\n";
    return 0;
}

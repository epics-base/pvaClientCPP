/*testEasyMonitorData.cpp */
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

#include <epicsUnitTest.h>
#include <testMain.h>

#include <pv/easyPVA.h>
#include <pv/bitSet.h>

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::easyPVA;

static EasyPVAPtr easyPVA = EasyPVA::create();
static FieldCreatePtr fieldCreate = getFieldCreate();
static StandardFieldPtr standardField = getStandardField();
static PVDataCreatePtr pvDataCreate = getPVDataCreate();


void testDouble()
{
    cout << "\nstarting testDouble\n";
    StructureConstPtr structure =
       fieldCreate->createFieldBuilder()->
            add("alarm",standardField->alarm()) ->
            add("timeStamp",standardField->timeStamp()) ->
            add("value",pvDouble) ->
            createStructure();

    EasyMonitorDataPtr easyData = EasyMonitorData::create(structure);
    MonitorElementPtr monitorElement(new MonitorElement(pvDataCreate->createPVStructure(easyData->getStructure())));
    easyData->setData(monitorElement);
    PVDoublePtr pvDouble = easyData->getPVStructure()->getSubField<PVDouble>("value");
    size_t valueOffset = pvDouble->getFieldOffset();
    BitSetPtr change = easyData->getChangedBitSet();
    pvDouble->put(5.0);
    change->set(pvDouble->getFieldOffset());
    testOk(change->cardinality()==1,"num set bits 1");
    testOk(change->get(valueOffset)==true,"value changed");
    testOk(easyData->hasValue()==true,"hasValue");
    testOk(easyData->isValueScalar()==true,"isValueScalar");
    testOk(easyData->isValueScalarArray()==false,"isValueScalarArray");
    bool result;
    result = false;
    if(easyData->getValue()) result = true;
    testOk(result==true,"getValue");
    result = false;
    if(easyData->getScalarValue()) result = true;
    testOk(result==true,"getScalarValue");
    try {
        easyData->getArrayValue();
    } catch (std::runtime_error e) {
        cout << "getArrayValue " << e.what() << endl;
    }
    try {
        easyData->getScalarArrayValue();
    } catch (std::runtime_error e) {
        cout << " getScalarArrayValue " << e.what() << endl;
    }
    cout << "as double " << easyData->getDouble() << endl;
    cout << "as string " << easyData->getString() << endl;
    try {
        shared_vector<const double> value = easyData->getDoubleArray();
    } catch (std::runtime_error e) {
        cout << " getDoubleArray " << e.what() << endl;
    }
    try {
        shared_vector<const string> value = easyData->getStringArray();
    } catch (std::runtime_error e) {
        cout << " getStringArray " << e.what() << endl;
    }
}

void testDoubleArray()
{
    cout << "\nstarting testDoubleArray\n";
    StructureConstPtr structure =
       fieldCreate->createFieldBuilder()->
            add("alarm",standardField->alarm()) ->
            add("timeStamp",standardField->timeStamp()) ->
            addArray("value",pvDouble) ->
            createStructure();

    EasyMonitorDataPtr easyData = EasyMonitorData::create(structure);
    MonitorElementPtr monitorElement(new MonitorElement(pvDataCreate->createPVStructure(easyData->getStructure())));
    easyData->setData(monitorElement);
    PVDoubleArrayPtr pvalue = easyData->getPVStructure()->getSubField<PVDoubleArray>("value");
    BitSetPtr change = easyData->getChangedBitSet();
    size_t valueOffset = pvalue->getFieldOffset();
    size_t len = 5;
    shared_vector<double> value(len);
    for(size_t i=0; i<len; ++i) value[i] = i*10.0;
    pvalue->replace(freeze(value));
    change->set(valueOffset);
    testOk(change->cardinality()==1,"num set bits 1");
    testOk(change->get(valueOffset)==true,"value changed");
    testOk(easyData->hasValue()==true,"hasValue");
    testOk(easyData->isValueScalar()==false,"isValueScalar");
    testOk(easyData->isValueScalarArray()==true,"isValueScalarArray");
    bool result;
    result = false;
    if(easyData->getValue()) result = true;
    testOk(result==true,"getValue");
    result = false;
    if(easyData->getArrayValue()) result = true;
    testOk(result==true,"getArrayValue");
    result = false;
    if(easyData->getScalarArrayValue()) result = true;
    testOk(result==true,"getScalarValue");
    try {
        easyData->getScalarValue();
    } catch (std::runtime_error e) {
        cout << " getScalarValue " << e.what() << endl;
    }
    try {
        cout << "as double " << easyData->getDouble() << endl;
    } catch (std::runtime_error e) {
        cout << " getDouble " << e.what() << endl;
    }
    try {
        string val = easyData->getString();
    } catch (std::runtime_error e) {
        cout << " getString " << e.what() << endl;
    }
    cout << "as doubleArray " << easyData->getDoubleArray() << endl;
    try {
        shared_vector<const string> value = easyData->getStringArray();
    } catch (std::runtime_error e) {
        cout << " getStringArray " << e.what() << endl;
    }
}

MAIN(testEasyMonitorData)
{
    cout << "\nstarting testEasyMonitorData\n";
    testPlan(15);
    testDouble();
    testDoubleArray();
    return 0;
}


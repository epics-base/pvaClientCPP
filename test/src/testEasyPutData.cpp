/*testEasyData.cpp */
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

static void testPostPut()
{
    cout << "\nstarting testPostPut\n";
    StructureConstPtr structure =
       fieldCreate->createFieldBuilder()->
            add("alarm",standardField->alarm()) ->
            add("timeStamp",standardField->timeStamp()) ->
            addNestedStructure("power") ->
               add("value",pvDouble) ->
               add("alarm",standardField->alarm()) ->
               endNested()->
            addNestedStructure("voltage") ->
               add("value",pvDouble) ->
               add("alarm",standardField->alarm()) ->
               endNested()->
            addNestedStructure("current") ->
               add("value",pvDouble) ->
               add("alarm",standardField->alarm()) ->
               endNested()->
            createStructure();

    EasyPutDataPtr easyData = EasyPutData::create(structure);
    PVStructurePtr pvStructure = easyData->getPVStructure();
    BitSetPtr change = easyData->getBitSet();
    PVDoublePtr powerValue = pvStructure->getSubField<PVDouble>("power.value");
    PVDoublePtr voltageValue = pvStructure->getSubField<PVDouble>("voltage.value");
    PVDoublePtr currentValue = pvStructure->getSubField<PVDouble>("current.value");
    size_t powerOffset = powerValue->getFieldOffset();
    size_t voltageOffset = voltageValue->getFieldOffset();
    size_t currentOffset = currentValue->getFieldOffset();
    change->clear();
    powerValue->put(1.0);
    voltageValue->put(2.0);
    currentValue->put(.5);
    cout << "changed\n";
    cout  << easyData->showChanged(cout) << endl;
    testOk(change->cardinality()==3,"num set bits 3");
    testOk(change->get(powerOffset)==true,"power changed");
    testOk(change->get(voltageOffset)==true,"voltage changed");
    testOk(change->get(currentOffset)==true,"current changed");
}

void testDouble()
{
    cout << "\nstarting testDouble\n";
    StructureConstPtr structure =
       fieldCreate->createFieldBuilder()->
            add("alarm",standardField->alarm()) ->
            add("timeStamp",standardField->timeStamp()) ->
            add("value",pvDouble) ->
            createStructure();

    EasyPutDataPtr easyData = EasyPutData::create(structure);
    PVDoublePtr pvDouble = easyData->getPVStructure()->getSubField<PVDouble>("value");
    pvDouble->put(5.0);
    BitSetPtr change = easyData->getBitSet();
    size_t valueOffset = pvDouble->getFieldOffset();
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
    easyData->putDouble(5.0);
    easyData->putString("1e5");
    try {
        size_t len = 2;
        shared_vector<double> val(len);
        for(size_t i=0; i<len; ++i) val[i] = (i+1)*10.0;
        easyData->putDoubleArray(freeze(val));
    } catch (std::runtime_error e) {
        cout << " putDoubleArray " << e.what() << endl;
    }
    try {
        size_t len = 2;
        shared_vector<string> val(len);
        val[0] = "one"; val[1] = "two";
        easyData->putStringArray(freeze(val));
    } catch (std::runtime_error e) {
        cout << " putStringArray " << e.what() << endl;
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

    EasyPutDataPtr easyData = EasyPutData::create(structure);
    PVDoubleArrayPtr pvalue = easyData->getPVStructure()->getSubField<PVDoubleArray>("value");
    size_t len = 5;
    shared_vector<double> value(len);
    for(size_t i=0; i<len; ++i) value[i] = i*10.0;
    pvalue->replace(freeze(value));
    BitSetPtr change = easyData->getBitSet();
    size_t valueOffset = pvalue->getFieldOffset();
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
    try {
        easyData->putDouble(5.0);
    } catch (std::runtime_error e) {
        cout << " putDouble " << e.what() << endl;
    }
    try {
        easyData->putString("1e5");
    } catch (std::runtime_error e) {
        cout << " putString " << e.what() << endl;
    }
    value = shared_vector<double>(len);
    for(size_t i=0; i<len; ++i) value[i] = (i+1)* 2;
    easyData->putDoubleArray(freeze(value));
    cout << "as doubleArray " << easyData->getDoubleArray() << endl;
    try {
        size_t len = 2;
        shared_vector<string> val(len);
        val[0] = "one"; val[1] = "two";
        easyData->putStringArray(freeze(val));
        cout << "as stringArray " << val << endl;
    } catch (std::runtime_error e) {
        cout << " putStringArray " << e.what() << endl;
    }
}

MAIN(testEasyPutData)
{
    cout << "\nstarting testEasyPutData\n";
    testPlan(19);
    testPostPut();
    testDouble();
    testDoubleArray();
    return 0;
}

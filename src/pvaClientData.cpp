/* pvaClientData.cpp */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvData is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
/**
 * @author mrk
 * @date 2019.04
 */

#include <typeinfo>
#include <sstream>

#include <pv/createRequest.h>
#include <pv/convert.h>

#define epicsExportSharedSymbols

#include <pv/pvaClient.h>

using std::tr1::static_pointer_cast;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace std;

namespace epics { namespace pvaClient {


typedef std::tr1::shared_ptr<PVArray> PVArrayPtr;
static ConvertPtr convert = getConvert();
static string noStructure("no pvStructure ");
static string noValue("no value field");
static string noScalar("value is not a scalar");
static string notCompatibleScalar("value is not a compatible scalar");
static string noArray("value is not an array");
static string noScalarArray("value is not a scalarArray");
static string notDoubleArray("value is not a doubleArray");
static string notStringArray("value is not a stringArray");
static string noAlarm("no alarm");
static string noTimeStamp("no timeStamp");

PvaClientDataPtr PvaClientData::create(StructureConstPtr const & structure)
{
    if(PvaClient::getDebug()) {
        cout << "PvaClientData::create"
           << endl;
    }
    PvaClientDataPtr epv(new PvaClientData(structure));
    return epv;
}

PvaClientData::PvaClientData(StructureConstPtr const & structure)
: structure(structure)
{
}

void PvaClientData::checkValue()
{
    if(PvaClient::getDebug()) {
        cout << "PvaClientData::checkValue"
           << endl;
    }
    if(pvValue) return;
    throw std::runtime_error(messagePrefix + noValue);
}

void PvaClientData::setMessagePrefix(std::string const & value)
{
    messagePrefix = value + " ";
}

StructureConstPtr PvaClientData::getStructure()
{
    return structure;
}

PVStructurePtr PvaClientData::getPVStructure()
{
    if(pvStructure) return pvStructure;
    throw std::runtime_error(messagePrefix + noStructure);
}

BitSetPtr PvaClientData::getChangedBitSet()
{
    if(bitSet)return bitSet;
    throw std::runtime_error(messagePrefix + noStructure);
}

std::ostream & PvaClientData::showChanged(std::ostream & out)
{
    if(!bitSet) throw std::runtime_error(messagePrefix + noStructure);
    size_t nextSet = bitSet->nextSetBit(0);
    PVFieldPtr pvField;
    while(nextSet!=string::npos) {
        if(nextSet==0) {
             pvField = pvStructure;
        } else {
              pvField = pvStructure->getSubField(nextSet);
        }
        string name = pvField->getFullName();
        out << name << " = " << pvField << endl;
        nextSet = bitSet->nextSetBit(nextSet+1);
    }
    return out;
}

void PvaClientData::setData(
    PVStructurePtr const & pvStructureFrom,
    BitSetPtr const & bitSetFrom)
{
    if(PvaClient::getDebug()) {
        cout << "PvaClientData::setData"
           << endl;
    }
   pvStructure = pvStructureFrom;
   bitSet = bitSetFrom;
   pvValue = pvStructure->getSubField("value");
   if(pvValue) return;
   // look for first field named value or is scalar or scalarArray
   PVStructurePtr pvStructure = pvStructureFrom;
   while(true) {
       PVFieldPtr pvField(pvStructure->getPVFields()[0]);
       if((pvField->getFieldName().compare("value")) == 0) {
           pvValue = pvField;
           return;
       }
       PVScalarPtr pvScalar = static_pointer_cast<PVScalar>(pvField);
       if(pvScalar) {
           pvValue = pvField;
           return;
       }
       PVScalarArrayPtr pvScalarArray = static_pointer_cast<PVScalarArray>(pvField);
       if(pvScalarArray) {
           pvValue = pvField;
           return;
       }
       PVStructurePtr pvStructure = static_pointer_cast<PVStructure>(pvField);
       if(!pvStructure) break;
   }
   messagePrefix = "did not find a field named value or a field that is a scalar or scalar array";
}

bool PvaClientData::hasValue()
{
    if(!pvValue) return false;
    return true;
}

bool PvaClientData::isValueScalar()
{
    if(!pvValue) return false;
    if(pvValue->getField()->getType()==scalar) return true;
    if((pvValue->getFieldName().compare("value")) != 0) return false;
    PVStructurePtr pvStructure = static_pointer_cast<PVStructure>(pvValue);
    while(true) {
        if(!pvStructure) break;
        PVFieldPtr pvField(pvStructure->getPVFields()[0]);
        PVScalarPtr pvScalar = static_pointer_cast<PVScalar>(pvField);
        if(pvScalar) {
           pvValue = pvField;
           return true;
        }
        PVStructurePtr pvStructure = static_pointer_cast<PVStructure>(pvField);
    }
    messagePrefix = "did not find a scalar field";
    return false;
}

bool PvaClientData::isValueScalarArray()
{
    if(!pvValue) return false;
    if(pvValue->getField()->getType()==scalarArray) return true;
    if((pvValue->getFieldName().compare("value")) != 0) return false;
    PVStructurePtr pvStructure = static_pointer_cast<PVStructure>(pvValue);
    while(true) {
        if(!pvStructure) break;
        PVFieldPtr pvField(pvStructure->getPVFields()[0]);
        PVScalarArrayPtr pvScalarArray = static_pointer_cast<PVScalarArray>(pvField);
        if(pvScalarArray) {
           pvValue = pvField;
           return true;
        }
        PVStructurePtr pvStructure = static_pointer_cast<PVStructure>(pvField);
    }
    messagePrefix = "did not find a scalarArray field";
    return false;
}

PVFieldPtr  PvaClientData::getValue()
{
   checkValue();
   return pvValue;
}

PVScalarPtr  PvaClientData::getScalarValue()
{
    checkValue();
    if(!isValueScalar()) throw std::runtime_error(messagePrefix + noScalar);
    PVScalarPtr pv = static_pointer_cast<PVScalar>(pvValue);
    if(!pv) throw std::runtime_error(messagePrefix + noScalar);
    return pv;
}

PVArrayPtr  PvaClientData::getArrayValue()
{
    checkValue();
    PVArrayPtr pv = pvStructure->getSubField<PVArray>("value");
    if(!pv) throw std::runtime_error(messagePrefix + noArray);
    return pv;
}

PVScalarArrayPtr  PvaClientData::getScalarArrayValue()
{
    checkValue();
    if(!isValueScalarArray()) throw std::runtime_error(messagePrefix + noScalarArray);
    PVScalarArrayPtr pv = static_pointer_cast<PVScalarArray>(pvValue);
    if(!pv) throw std::runtime_error(messagePrefix + noScalarArray);
    return pv;
}

double PvaClientData::getDouble()
{
    PVScalarPtr pvScalar = getScalarValue();
    ScalarType scalarType = pvScalar->getScalar()->getScalarType();
    if(scalarType==pvDouble) {
        PVDoublePtr pvDouble = static_pointer_cast<PVDouble>(pvScalar);
        return pvDouble->get();
    }
    if(!ScalarTypeFunc::isNumeric(scalarType)) {
        throw std::runtime_error(messagePrefix + notCompatibleScalar);
    }
    return convert->toDouble(pvScalar);
}

string PvaClientData::getString()
{
    PVScalarPtr pvScalar = getScalarValue();
    return convert->toString(pvScalar);
}

shared_vector<const double> PvaClientData::getDoubleArray()
{
    PVScalarArrayPtr pvScalarArray = getScalarArrayValue();
    PVDoubleArrayPtr pv = static_pointer_cast<PVDoubleArray>(pvScalarArray);
    if(!pv) throw std::runtime_error(messagePrefix + notDoubleArray);
    return pv->view();   
}

shared_vector<const string> PvaClientData::getStringArray()
{
    PVScalarArrayPtr pvScalarArray = getScalarArrayValue();
    PVStringArrayPtr pv = static_pointer_cast<PVStringArray>(pvScalarArray);
    if(!pv) throw std::runtime_error(messagePrefix + notStringArray);
    return pv->view();   
}


Alarm PvaClientData::getAlarm()
{
   if(!pvStructure) throw new std::runtime_error(messagePrefix + noStructure);
   PVStructurePtr pvs = pvStructure->getSubField<PVStructure>("alarm");
   if(!pvs) throw std::runtime_error(messagePrefix + noAlarm);
   pvAlarm.attach(pvs);
   if(pvAlarm.isAttached()) {
       Alarm alarm;
       pvAlarm.get(alarm);
       pvAlarm.detach();
       return alarm;
   }
   throw std::runtime_error(messagePrefix + noAlarm);
}

TimeStamp PvaClientData::getTimeStamp()
{
   if(!pvStructure) throw new std::runtime_error(messagePrefix + noStructure);
   PVStructurePtr pvs = pvStructure->getSubField<PVStructure>("timeStamp");
   if(!pvs) throw std::runtime_error(messagePrefix + noTimeStamp);
   pvTimeStamp.attach(pvs);
   if(pvTimeStamp.isAttached()) {
       TimeStamp timeStamp;
       pvTimeStamp.get(timeStamp);
       pvTimeStamp.detach();
       return timeStamp;
   }
   throw std::runtime_error(messagePrefix + noTimeStamp);
}

}}

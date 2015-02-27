/* easyPVStructure.cpp */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvData is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
/**
 * @author mrk
 * @date 2015.02
 */
#define epicsExportSharedSymbols

#include <typeinfo>

#include <sstream>
#include <pv/easyPVA.h>
#include <pv/createRequest.h>
#include <pv/convert.h>


using std::tr1::static_pointer_cast;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace std;

namespace epics { namespace easyPVA {

typedef std::tr1::shared_ptr<epics::pvData::PVArray> PVArrayPtr;
static StructureConstPtr nullStructure;
static PVStructurePtr nullPVStructure;
static ConvertPtr convert = getConvert();
static Status statusOK(Status::Ok);
static Status statusDestroyed(Status::STATUSTYPE_ERROR,"was destroyed");
static Status statusNoPVStructure(Status::STATUSTYPE_ERROR,"setPVStructure not called");
static Status statusNoValue(Status::STATUSTYPE_ERROR,"no value field");
static Status statusNoScalar(Status::STATUSTYPE_ERROR,"value is not a scalar");
static Status statusMismatchedScalar(Status::STATUSTYPE_ERROR,"value is not a compatible scalar");
static Status statusNoArray(Status::STATUSTYPE_ERROR,"value is not a array");
static Status statusNoScalarArray(Status::STATUSTYPE_ERROR,"value is not a scalarArray");
static Status statusMismatchedScalarArray(Status::STATUSTYPE_ERROR,"value is not a compatible scalarArray");
static Status statusNoAlarm(Status::STATUSTYPE_ERROR,"no alarm field");
static Status statusNoTimeStamp(Status::STATUSTYPE_ERROR,"no timeStamp field");

class epicsShareClass EasyPVStructureImpl :
    public EasyPVStructure,
    public std::tr1::enable_shared_from_this<EasyPVStructureImpl>
{
public:
    EasyPVStructureImpl();
    ~EasyPVStructureImpl(){}
    virtual void setMessagePrefix(std::string const & value);
    virtual void setPVStructure(PVStructurePtr const & pvStructure);
    virtual PVStructurePtr getPVStructure();
    virtual Alarm getAlarm();
    virtual TimeStamp getTimeStamp();
    virtual bool hasValue();
    virtual bool isValueScalar();
    virtual bool isValueScalarArray() ;
    virtual PVFieldPtr getValue();
    virtual PVScalarPtr getScalarValue();
    virtual std::tr1::shared_ptr<PVArray> getArrayValue();
    virtual std::tr1::shared_ptr<PVScalarArray> getScalarArrayValue();
    virtual bool getBoolean();
    virtual int8 getByte();
    virtual int16 getShort();
    virtual int32 getInt();
    virtual int64 getLong();
    virtual uint8 getUByte();
    virtual uint16 getUShort();
    virtual uint32 getUInt();
    virtual uint64 getULong();
    virtual float getFloat();
    virtual double getDouble();
    virtual std::string getString();
    
    virtual shared_vector<boolean>  getBooleanArray();
    virtual shared_vector<int8>  getByteArray();
    virtual shared_vector<int16>  getShortArray();
    virtual shared_vector<int32>  getIntArray();
    virtual shared_vector<int64>  getLongArray();
    virtual shared_vector<uint8>  getUByteArray();
    virtual shared_vector<uint16>  getUShortArray();
    virtual shared_vector<uint32>  getUIntArray();
    virtual shared_vector<uint64>  getULongArray();
    virtual shared_vector<float>  getFloatArray();
    virtual shared_vector<double>  getDoubleArray();
    virtual shared_vector<string>  getStringArray();
    
    EasyPVStructurePtr getPtrSelf()
    {
        return shared_from_this();
    }
private:
    void checkPVStructure();
    void checkValue();
    bool checkOverflow(ScalarType source,ScalarType dest);
    PVScalarPtr checkScalar(ScalarType scalarType);
    PVScalarArrayPtr checkScalarArray(ScalarType elementType);

    string messagePrefix;
    PVStructurePtr pvStructure;
    PVFieldPtr pvValue;
    PVAlarm pvAlarm;
    Alarm alarm;
    PVTimeStamp pvTimeStamp;
    TimeStamp timeStamp;
};

EasyPVStructureImpl::EasyPVStructureImpl()
{}

EasyPVStructurePtr EasyPVStructureFactory::createEasyPVStructure()
{
    EasyPVStructurePtr epv(new EasyPVStructureImpl());
    return epv;
}

void EasyPVStructureImpl::checkPVStructure()
{
    if(pvStructure) return;
    throw std::runtime_error(messagePrefix + statusNoPVStructure.getMessage());
    
}

void EasyPVStructureImpl::checkValue()
{
    if(pvValue) return;
    throw std::runtime_error(messagePrefix + statusNoValue.getMessage());
}

bool EasyPVStructureImpl::checkOverflow(ScalarType source,ScalarType dest)
{
    if(dest==pvFloat||dest==pvDouble) return true;
    if(!ScalarTypeFunc::isInteger(source) && !ScalarTypeFunc::isUInteger(source)) return false;
    if(ScalarTypeFunc::isUInteger(dest)) {
        if(ScalarTypeFunc::isUInteger(source)) {
             if(dest>=source) return true;
             return false;
        }
        if(ScalarTypeFunc::isInteger(source)) {
             if(dest>=(source+4)) return true;
             return false;
        }
        return false;
    }
    if(ScalarTypeFunc::isInteger(dest)) {
        if(ScalarTypeFunc::isUInteger(source)) {
             if(dest>(source-4)) return true;
             return false;
        }
        if(ScalarTypeFunc::isInteger(source)) {
             if(dest>=source) return true;
             return false;
        }
        return false;
    }
    return false;
}

PVScalarPtr EasyPVStructureImpl::checkScalar(ScalarType scalarType)
{
    checkPVStructure();
    checkValue();
    PVScalarPtr pv = pvStructure->getSubField<PVScalar>("value");
    if(!pv) {
        throw std::runtime_error(messagePrefix + statusNoScalar.getMessage());
    }
    ScalarType type = pv->getScalar()->getScalarType();
    if((scalarType==pvBoolean && type==pvBoolean)
    || (scalarType==pvString && type==pvString)) return pv;
    if((ScalarTypeFunc::isNumeric(type) && ScalarTypeFunc::isNumeric(scalarType))
    && checkOverflow(type,scalarType)) return pv;
    stringstream ss;
    ss << messagePrefix << statusMismatchedScalar.getMessage();
    ss << " source " << type << " dest " << scalarType;
    throw std::runtime_error(ss.str());
}

PVScalarArrayPtr EasyPVStructureImpl::checkScalarArray(ScalarType elementType)
{
    checkPVStructure();
    checkValue();
    PVScalarArrayPtr pv = pvStructure->getSubField<PVScalarArray>("value");
    if(!pv) {
        throw std::runtime_error(messagePrefix + statusNoScalarArray.getMessage());
    }
    ScalarType type = pv->getScalarArray()->getElementType();
    if((elementType==pvBoolean && type==pvBoolean)
    || (elementType==pvBoolean && type==pvBoolean)) return pv;
    if((ScalarTypeFunc::isNumeric(type) && ScalarTypeFunc::isNumeric(elementType))
    && checkOverflow(type,elementType)) return pv;
    throw std::runtime_error(messagePrefix + statusMismatchedScalarArray.getMessage());
}

void EasyPVStructureImpl::setMessagePrefix(string const & value)
{
     messagePrefix = value;
     if(value.size()>0) messagePrefix += " ";
}

void EasyPVStructureImpl::setPVStructure(PVStructurePtr const & pvStructure)
{
    this->pvStructure = pvStructure;
    pvValue = pvStructure->getSubField("value");
}

PVStructurePtr EasyPVStructureImpl::getPVStructure()
{
    checkPVStructure();
    return pvStructure;
}

PVFieldPtr  EasyPVStructureImpl::getValue()
{
   checkValue();
   return pvValue;
}

PVScalarPtr  EasyPVStructureImpl::getScalarValue()
{
    checkValue();
    PVScalarPtr pv = pvStructure->getSubField<PVScalar>("value");
    if(!pv) {
        throw std::runtime_error(messagePrefix + statusNoScalar.getMessage());
    }
    return pv;
}

PVArrayPtr  EasyPVStructureImpl::getArrayValue()
{
    checkValue();
    PVArrayPtr pv = pvStructure->getSubField<PVArray>("value");
    if(!pv) {
        throw std::runtime_error(messagePrefix + statusNoArray.getMessage());
    }
    return pv;
}

PVScalarArrayPtr  EasyPVStructureImpl::getScalarArrayValue()
{
    checkValue();
    PVScalarArrayPtr pv = pvStructure->getSubField<PVScalarArray>("value");
    if(!pv) {
        throw std::runtime_error(messagePrefix + statusNoScalarArray.getMessage());
    }
    return pv;
}

Alarm EasyPVStructureImpl::getAlarm()
{
    Alarm alarm;
    alarm.setSeverity(undefinedAlarm);
    alarm.setStatus(undefinedStatus);
    alarm.setMessage("no alarm field");
    if(!pvStructure) return alarm;
    PVStructurePtr xxx = pvStructure->getSubField<PVStructure>("alarm");
    if(xxx) {
        pvAlarm.attach(xxx);
        if(pvAlarm.isAttached()) {
             pvAlarm.get(alarm);
             pvAlarm.detach();
        }
    }
    return alarm;;
}

TimeStamp EasyPVStructureImpl::getTimeStamp()
{
    TimeStamp timeStamp;
    if(!pvStructure) return timeStamp;
    PVStructurePtr xxx = pvStructure->getSubField<PVStructure>("timeStamp");
    if(xxx) {
        pvTimeStamp.attach(xxx);
        if(pvTimeStamp.isAttached()) {
             pvTimeStamp.get(timeStamp);
             pvTimeStamp.detach();
        }
    }
    return timeStamp;;
}

bool EasyPVStructureImpl::hasValue()
{
    if(!pvValue) return false;
    return true;
}

bool EasyPVStructureImpl::isValueScalar()
{
    if(!pvValue) return false;
    if(pvValue->getField()->getType()==scalar) return true;
    return false;
}

bool EasyPVStructureImpl::isValueScalarArray()
{
    if(!pvValue) return false;
    if(pvValue->getField()->getType()==scalarArray) return true;
    return false;
}

bool EasyPVStructureImpl::getBoolean()
{
    PVScalarPtr pvScalar = checkScalar(pvBoolean);
    PVBooleanPtr pv = static_pointer_cast<PVBoolean>(pvScalar);
    return pv->get();
}

int8 EasyPVStructureImpl::getByte()
{
    PVScalarPtr pvScalar = checkScalar(pvByte);
    return convert->toByte(pvScalar);
}

uint8 EasyPVStructureImpl::getUByte()
{
    PVScalarPtr pvScalar = checkScalar(pvUByte);
    return convert->toUByte(pvScalar);
}

int16 EasyPVStructureImpl::getShort()
{
    PVScalarPtr pvScalar = checkScalar(pvShort);
    return convert->toShort(pvScalar);
}

uint16 EasyPVStructureImpl::getUShort()
{
    PVScalarPtr pvScalar = checkScalar(pvUShort);
    return convert->toUShort(pvScalar);
}

int32 EasyPVStructureImpl::getInt()
{
    PVScalarPtr pvScalar = checkScalar(pvInt);
    return convert->toInt(pvScalar);
}

uint32 EasyPVStructureImpl::getUInt()
{
    PVScalarPtr pvScalar = checkScalar(pvUInt);
    return convert->toUInt(pvScalar);
}

int64 EasyPVStructureImpl::getLong()
{
    PVScalarPtr pvScalar = checkScalar(pvLong);
    return convert->toLong(pvScalar);
}

uint64 EasyPVStructureImpl::getULong()
{
    PVScalarPtr pvScalar = checkScalar(pvULong);
    return convert->toULong(pvScalar);
}

float EasyPVStructureImpl::getFloat()
{
    PVScalarPtr pvScalar = checkScalar(pvFloat);
    return convert->toFloat(pvScalar);
}

double EasyPVStructureImpl::getDouble()
{
    PVScalarPtr pvScalar = checkScalar(pvDouble);
    return convert->toDouble(pvScalar);
}

string EasyPVStructureImpl::getString()
{
    PVScalarPtr pvScalar = checkScalar(pvString);
    PVStringPtr pv = static_pointer_cast<PVString>(pvScalar);
    return pv->get();
}

shared_vector<boolean>   EasyPVStructureImpl::getBooleanArray()
{
    checkScalarArray(pvBoolean);
    PVBooleanArrayPtr pv = static_pointer_cast<PVBooleanArray>(pvValue);
    return pv->reuse();
}

template <typename T>
shared_vector<T> copy(PVScalarArrayPtr const & pvScalarArray)
{
     ScalarType elementType = pvScalarArray->getScalarArray()->getElementType();
     switch(elementType) {
         case pvBoolean :
             break;
         case pvByte :
             {
               PVByteArrayPtr pv = static_pointer_cast<PVByteArray>(pvScalarArray);
               shared_vector<int8> data = pv->reuse();
               shared_vector<T> to = shared_vector_convert<T,int8>(data);
               return to;
             }
         case pvShort :
             {
               PVShortArrayPtr pv = static_pointer_cast<PVShortArray>(pvScalarArray);
               shared_vector<int16> data = pv->reuse();
               shared_vector<T> to = shared_vector_convert<T,int16>(data);
               return to;
             }
         case pvInt :
             {
               PVIntArrayPtr pv = static_pointer_cast<PVIntArray>(pvScalarArray);
               shared_vector<int32> data = pv->reuse();
               shared_vector<T> to = shared_vector_convert<T,int32>(data);
               return to;
             }
         case pvLong :
             {
               PVLongArrayPtr pv = static_pointer_cast<PVLongArray>(pvScalarArray);
               shared_vector<int64> data = pv->reuse();
               shared_vector<T> to = shared_vector_convert<T,int64>(data);
               return to;
             }
         case pvUByte :
             {
               PVUByteArrayPtr pv = static_pointer_cast<PVUByteArray>(pvScalarArray);
               shared_vector<uint8> data = pv->reuse();
               shared_vector<T> to = shared_vector_convert<T,uint8>(data);
               return to;
             }
         case pvUShort :
             {
               PVUShortArrayPtr pv = static_pointer_cast<PVUShortArray>(pvScalarArray);
               shared_vector<uint16> data = pv->reuse();
               shared_vector<T> to = shared_vector_convert<T,uint16>(data);
               return to;
             }
         case pvUInt :
             {
               PVUIntArrayPtr pv = static_pointer_cast<PVUIntArray>(pvScalarArray);
               shared_vector<uint32> data = pv->reuse();
               shared_vector<T> to = shared_vector_convert<T,uint32>(data);
               return to;
             }
         case pvULong :
             {
               PVULongArrayPtr pv = static_pointer_cast<PVULongArray>(pvScalarArray);
               shared_vector<uint64> data = pv->reuse();
               shared_vector<T> to = shared_vector_convert<T,uint64>(data);
               return to;
             }
         case pvFloat :
             {
               PVFloatArrayPtr pv = static_pointer_cast<PVFloatArray>(pvScalarArray);
               shared_vector<float> data = pv->reuse();
               shared_vector<T> to = shared_vector_convert<T,float>(data);
               return to;
             }
         case pvDouble :
             {
               PVDoubleArrayPtr pv = static_pointer_cast<PVDoubleArray>(pvScalarArray);
               shared_vector<double> data = pv->reuse();
               shared_vector<T> to = shared_vector_convert<T,double>(data);
               return to;
             }
         case pvString :
             break;
    }
    return shared_vector<T>();
}

shared_vector<int8>   EasyPVStructureImpl::getByteArray()
{
    PVScalarArrayPtr pvScalarArray = checkScalarArray(pvByte);
    ScalarType scalarType = pvScalarArray->getScalarArray()->getElementType();
    if(scalarType==pvByte) {
        PVByteArrayPtr pv = static_pointer_cast<PVByteArray>(pvValue);
        return pv->reuse();
    }
    shared_vector<int8> xx = copy<int8>(pvScalarArray);
    return xx;
}

shared_vector<int16>   EasyPVStructureImpl::getShortArray()
{
    PVScalarArrayPtr pvScalarArray = checkScalarArray(pvShort);
    ScalarType scalarType = pvScalarArray->getScalarArray()->getElementType();
    if(scalarType==pvShort) {
        PVShortArrayPtr pv = static_pointer_cast<PVShortArray>(pvValue);
        return pv->reuse();
    }
    shared_vector<int16> xx = copy<int16>(pvScalarArray);
    return xx;
}

shared_vector<int32>   EasyPVStructureImpl::getIntArray()
{
    PVScalarArrayPtr pvScalarArray = checkScalarArray(pvInt);
    ScalarType scalarType = pvScalarArray->getScalarArray()->getElementType();
    if(scalarType==pvInt) {
        PVIntArrayPtr pv = static_pointer_cast<PVIntArray>(pvValue);
        return pv->reuse();
    }
    shared_vector<int32> xx = copy<int32>(pvScalarArray);
    return xx;
}

shared_vector<int64>   EasyPVStructureImpl::getLongArray()
{
    PVScalarArrayPtr pvScalarArray = checkScalarArray(pvLong);
    ScalarType scalarType = pvScalarArray->getScalarArray()->getElementType();
    if(scalarType==pvLong) {
        PVLongArrayPtr pv = static_pointer_cast<PVLongArray>(pvValue);
        return pv->reuse();
    }
    shared_vector<int64> xx = copy<int64>(pvScalarArray);
    return xx;
}

shared_vector<uint8>   EasyPVStructureImpl::getUByteArray()
{
    PVScalarArrayPtr pvScalarArray = checkScalarArray(pvUByte);
    ScalarType scalarType = pvScalarArray->getScalarArray()->getElementType();
    if(scalarType==pvUByte) {
        PVUByteArrayPtr pv = static_pointer_cast<PVUByteArray>(pvValue);
        return pv->reuse();
    }
    shared_vector<uint8> xx = copy<uint8>(pvScalarArray);
    return xx;
}

shared_vector<uint16>   EasyPVStructureImpl::getUShortArray()
{
    PVScalarArrayPtr pvScalarArray = checkScalarArray(pvUShort);
    ScalarType scalarType = pvScalarArray->getScalarArray()->getElementType();
    if(scalarType==pvUShort) {
        PVUShortArrayPtr pv = static_pointer_cast<PVUShortArray>(pvValue);
        return pv->reuse();
    }
    shared_vector<uint16> xx = copy<uint16>(pvScalarArray);
    return xx;
}

shared_vector<uint32>   EasyPVStructureImpl::getUIntArray()
{
    PVScalarArrayPtr pvScalarArray = checkScalarArray(pvUInt);
    ScalarType scalarType = pvScalarArray->getScalarArray()->getElementType();
    if(scalarType==pvUInt) {
        PVUIntArrayPtr pv = static_pointer_cast<PVUIntArray>(pvValue);
        return pv->reuse();
    }
    shared_vector<uint32> xx = copy<uint32>(pvScalarArray);
    return xx;
}

shared_vector<uint64>   EasyPVStructureImpl::getULongArray()
{
    PVScalarArrayPtr pvScalarArray = checkScalarArray(pvULong);
    ScalarType scalarType = pvScalarArray->getScalarArray()->getElementType();
    if(scalarType==pvULong) {
        PVULongArrayPtr pv = static_pointer_cast<PVULongArray>(pvValue);
        return pv->reuse();
    }
    shared_vector<uint64> xx = copy<uint64>(pvScalarArray);
    return xx;
}

shared_vector<float>   EasyPVStructureImpl::getFloatArray()
{
    checkScalarArray(pvFloat);
    PVFloatArrayPtr pv = static_pointer_cast<PVFloatArray>(pvValue);
    return pv->reuse();
    PVScalarArrayPtr pvScalarArray = checkScalarArray(pvFloat);
    ScalarType scalarType = pvScalarArray->getScalarArray()->getElementType();
    if(scalarType==pvFloat) {
        PVFloatArrayPtr pv = static_pointer_cast<PVFloatArray>(pvValue);
        return pv->reuse();
    }
    shared_vector<float> xx = copy<float>(pvScalarArray);
    return xx;
}

shared_vector<double>   EasyPVStructureImpl::getDoubleArray()
{
    PVScalarArrayPtr pvScalarArray = checkScalarArray(pvDouble);
    ScalarType scalarType = pvScalarArray->getScalarArray()->getElementType();
    if(scalarType==pvDouble) {
        PVDoubleArrayPtr pv = static_pointer_cast<PVDoubleArray>(pvValue);
        return pv->reuse();
    }
    shared_vector<double> xx = copy<double>(pvScalarArray);
    return xx;
}

shared_vector<string>   EasyPVStructureImpl::getStringArray()
{
    checkScalarArray(pvString);
    PVStringArrayPtr pv = static_pointer_cast<PVStringArray>(pvValue);
    return pv->reuse();
}


}}

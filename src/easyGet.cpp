/* easyGet.cpp */
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

#include <sstream>
#include <pv/event.h>
#include <pv/lock.h>
#include <pv/easyPVA.h>
#include <pv/createRequest.h>

using std::tr1::static_pointer_cast;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace std;

namespace epics { namespace easyPVA {

   

class epicsShareClass EasyGetImpl :
    public EasyGet,
    public std::tr1::enable_shared_from_this<EasyGetImpl>
{
public:
    EasyGetImpl(
        EasyPVAPtr const &pva,
        EasyChannelPtr const & easyChannel,
        Channel::shared_pointer const & channel,
        PVStructurePtr const &pvRequest);
    ~EasyGetImpl();
    // from ChannelGetRequester
    string getRequesterName();
    void message(string const & message,MessageType messageType);
    void channelGetConnect(
        const Status& status,
        ChannelGet::shared_pointer const & channelGet,
        StructureConstPtr const & structure);
    void getDone(
        const Status& status,
        ChannelGet::shared_pointer const & channelGet,
        PVStructurePtr const & pvStructure,
        BitSetPtr const & bitSet);

    // from EasyGet
    virtual void destroy();
    virtual void connect();
    virtual void issueConnect();
    virtual Status waitConnect();
    virtual void get();
    virtual void issueGet();
    virtual Status waitGet();
    virtual BitSetPtr getBitSet();
    // from EasyPVStructure
    virtual void setMessagePrefix(std::string const & value);
    virtual void setPVStructure(epics::pvData::PVStructurePtr const & pvStructure);
    virtual Alarm getAlarm();
    virtual TimeStamp getTimeStamp();
    virtual bool hasValue();
    virtual bool isValueScalar();
    virtual bool isValueScalarArray();
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
    virtual shared_vector<std::string>  getStringArray();
    virtual PVStructurePtr getPVStructure();
    EasyGetPtr getPtrSelf()
    {
        return shared_from_this();
    }
private:
    enum GetConnectState {connectIdle,connectActive,connected};

    EasyPVAPtr easyPVA;
    EasyChannelPtr easyChannel;
    Channel::shared_pointer channel;
    ChannelGetRequester::shared_pointer getRequester;
    PVStructurePtr pvRequest;
    Mutex mutex;
    Event waitForConnect;
    Event waitForGet;
    EasyPVStructurePtr easyPVStructure;
    string messagePrefix;

    bool isDestroyed;
    Status channelGetConnectStatus;
    Status channelGetStatus;
    ChannelGet::shared_pointer channelGet;
    BitSet::shared_pointer bitSet;
    
    GetConnectState connectState;

    enum GetState {getIdle,getActive,getComplete};
    GetState getState;
    bool getSuccess;
};

namespace easyGet {
class ChannelGetRequesterImpl : public ChannelGetRequester
{
    EasyGetImpl * easyGet;
public:
    ChannelGetRequesterImpl(EasyGetImpl * easyGet)
    : easyGet(easyGet) {}
    virtual string getRequesterName()
    {return easyGet->getRequesterName();}
    virtual void message(string const & message,MessageType messageType)
    {easyGet->message(message,messageType);}
    virtual void channelGetConnect(
        const Status& status,
        ChannelGet::shared_pointer const & channelGet,
        StructureConstPtr const & structure)
    {easyGet->channelGetConnect(status,channelGet,structure);}
    virtual void getDone(
        const Status& status,
        ChannelGet::shared_pointer const & channelGet,
        PVStructurePtr const & pvStructure,
        BitSetPtr const & bitSet)
    {easyGet->getDone(status,channelGet,pvStructure,bitSet);}
};
} // namespace easyGet

using namespace epics::easyPVA::easyGet;

EasyGetImpl::EasyGetImpl(
        EasyPVAPtr const &pva,
        EasyChannelPtr const & easyChannel,
        Channel::shared_pointer const & channel,
        PVStructurePtr const &pvRequest)
: easyPVA(pva),
  easyChannel(easyChannel),
  channel(channel),
  pvRequest(pvRequest),
  easyPVStructure(pva->createEasyPVStructure()),
  isDestroyed(false),
  connectState(connectIdle),
  getState(getIdle)
{
    easyPVStructure->setMessagePrefix("channel " + channel->getChannelName() + " EasyGet");
}

EasyGetImpl::~EasyGetImpl()
{
    destroy();
}


// from ChannelGetRequester
string EasyGetImpl::getRequesterName()
{
     return easyPVA->getRequesterName();
}

void EasyGetImpl::message(string const & message,MessageType messageType)
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    easyPVA->message(message, messageType);
}

void EasyGetImpl::channelGetConnect(
    const Status& status,
    ChannelGet::shared_pointer const & channelGet,
    StructureConstPtr const & structure)
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    channelGetConnectStatus = status;
    this->channelGet = channelGet;
    if(status.isOK()) {
        connectState = connected;
    } else {
        connectState = connectIdle;
    }
    waitForConnect.signal();
    
}

void EasyGetImpl::getDone(
    const Status& status,
    ChannelGet::shared_pointer const & channelGet,
    PVStructurePtr const & pvStructure,
    BitSetPtr const & bitSet)
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    easyPVStructure->setPVStructure(pvStructure);
    this->bitSet = bitSet;
    channelGetStatus = status;
    if(status.isOK()) {
        getState = getComplete;
    } else {
        getState = getIdle;
    }
    waitForGet.signal();
}


// from EasyGet
void EasyGetImpl::destroy()
{
    {
        Lock xx(mutex);
        if(isDestroyed) return;
        isDestroyed = true;
    }
    if(channelGet) channelGet->destroy();
    channelGet.reset();
}

void EasyGetImpl::connect()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    issueConnect();
    Status status = waitConnect();
    if(status.isOK()) return;
    stringstream ss;
    ss << "channel " << channel->getChannelName() << " EasyGet::connect " << status.getMessage();
    throw std::runtime_error(ss.str());
}

void EasyGetImpl::issueConnect()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    if(connectState!=connectIdle) {
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " easyGet already connected ";
        throw std::runtime_error(ss.str());
    }
    getRequester = ChannelGetRequester::shared_pointer(new ChannelGetRequesterImpl(this));

    connectState = connectActive;
    channelGet = channel->createChannelGet(getRequester,pvRequest);
}

Status EasyGetImpl::waitConnect()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    if(connectState!=connectActive) {
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " easyGet illegal connect state ";
        throw std::runtime_error(ss.str());
    }
    waitForConnect.wait();
    if(connectState==connected) return Status::Ok;
    return Status(Status::STATUSTYPE_ERROR,channelGetConnectStatus.getMessage());
}

void EasyGetImpl::get()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    issueGet();
    Status status = waitGet();
    if(status.isOK()) return;
    stringstream ss;
    ss << "channel " << channel->getChannelName() << " EasyGet::get " << status.getMessage();
    throw std::runtime_error(ss.str());
}

void EasyGetImpl::issueGet()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    if(connectState==connectIdle) connect();
    if(getState!=getIdle) {
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " EasyGet::issueGet get aleady active ";
        throw std::runtime_error(ss.str());
    }
    getState = getActive;
    channelGet->get();
}

Status EasyGetImpl::waitGet()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    if(getState!=getActive){
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " EasyGet::waitGet llegal get state";
        throw std::runtime_error(ss.str());
    }
    waitForGet.wait();
    if(getState==getComplete) {
        getState = getIdle;
        return Status::Ok;
    }
    return Status(Status::STATUSTYPE_ERROR,channelGetStatus.getMessage());
}

BitSetPtr EasyGetImpl::getBitSet()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return bitSet;
}

void EasyGetImpl::setMessagePrefix(string const & value)
{
     messagePrefix = value;
     if(value.size()>0) messagePrefix += " ";
}

void EasyGetImpl::setPVStructure(epics::pvData::PVStructurePtr const & pvStructure)
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    throw std::runtime_error("easyGet does not implement setPVStructure");
}

Alarm EasyGetImpl::getAlarm()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getAlarm();
}

TimeStamp EasyGetImpl::getTimeStamp()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getTimeStamp();
}

bool EasyGetImpl::hasValue()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->hasValue();
}

bool EasyGetImpl::isValueScalar()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->isValueScalar();
}

bool EasyGetImpl::isValueScalarArray()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->isValueScalarArray();
}

PVFieldPtr EasyGetImpl::getValue()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getValue();
}

PVScalarPtr EasyGetImpl::getScalarValue()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getScalarValue();
}

std::tr1::shared_ptr<PVArray> EasyGetImpl::getArrayValue()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getArrayValue();
}

std::tr1::shared_ptr<PVScalarArray> EasyGetImpl::getScalarArrayValue()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getScalarArrayValue();
}

bool EasyGetImpl::getBoolean()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getBoolean();
}

int8 EasyGetImpl::getByte()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getByte();
}

int16 EasyGetImpl::getShort()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getShort();
}

int32 EasyGetImpl::getInt()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getInt();
}

int64 EasyGetImpl::getLong()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getLong();
}

uint8 EasyGetImpl::getUByte()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getUByte();
}

uint16 EasyGetImpl::getUShort()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getUShort();
}

uint32 EasyGetImpl::getUInt()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getUInt();
}

uint64 EasyGetImpl::getULong()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getULong();
}

float EasyGetImpl::getFloat()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getFloat();
}

double EasyGetImpl::getDouble()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->isValueScalar();
}

std::string EasyGetImpl::getString()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getString();
}


shared_vector<boolean>  EasyGetImpl::getBooleanArray()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getBooleanArray();
}

shared_vector<int8>  EasyGetImpl::getByteArray()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getByteArray();
}

shared_vector<int16>  EasyGetImpl::getShortArray()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getShortArray();
}

shared_vector<int32>  EasyGetImpl::getIntArray()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getIntArray();
}

shared_vector<int64>  EasyGetImpl::getLongArray()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getLongArray();
}

shared_vector<uint8>  EasyGetImpl::getUByteArray()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getUByteArray();
}

shared_vector<uint16>  EasyGetImpl::getUShortArray()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getUShortArray();
}

shared_vector<uint32>  EasyGetImpl::getUIntArray()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getUIntArray();
}

shared_vector<uint64>  EasyGetImpl::getULongArray()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getULongArray();
}

shared_vector<float>  EasyGetImpl::getFloatArray()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getFloatArray();
}

shared_vector<double>  EasyGetImpl::getDoubleArray()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getDoubleArray();
}

shared_vector<std::string>  EasyGetImpl::getStringArray()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getStringArray();
}

PVStructurePtr EasyGetImpl::getPVStructure()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    return easyPVStructure->getPVStructure();
}

EasyGetPtr EasyGetFactory::createEasyGet(
        EasyPVAPtr const &pva,
        EasyChannelPtr const & easyChannel,
        Channel::shared_pointer const & channel,
        PVStructurePtr const &pvRequest)
{
    EasyGetPtr epv(new EasyGetImpl(pva,easyChannel,channel,pvRequest));
    return epv;
}

}}

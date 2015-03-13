/* easyPutGet.cpp */
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
#include <pv/easyPVA.h>

using std::tr1::static_pointer_cast;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace std;

namespace epics { namespace easyPVA {

class ChannelPutGetRequesterImpl : public ChannelPutGetRequester
{
    EasyPutGet * easyPutGet;
public:
    ChannelPutGetRequesterImpl(EasyPutGet * easyPutGet)
    : easyPutGet(easyPutGet) {}
    string getRequesterName()
    {return easyPutGet->getRequesterName();}
    void message(string const & message,MessageType messageType)
    {easyPutGet->message(message,messageType);}
    void channelPutGetConnect(
        const epics::pvData::Status& status,
        epics::pvAccess::ChannelPutGet::shared_pointer const & channelPutGet,
        epics::pvData::StructureConstPtr const & putStructure,
        epics::pvData::StructureConstPtr const & getStructure)
    {
         easyPutGet->channelPutGetConnect(status,channelPutGet,putStructure,getStructure);
    }
    void putGetDone(
        const epics::pvData::Status& status,
        epics::pvAccess::ChannelPutGet::shared_pointer const & channelPutGet,
        epics::pvData::PVStructurePtr const & getPVStructure,
        epics::pvData::BitSetPtr const & getBitSet)
    {
        easyPutGet->putGetDone(status,channelPutGet,getPVStructure,getBitSet);
    }
    void getPutDone(
        const epics::pvData::Status& status,
        epics::pvAccess::ChannelPutGet::shared_pointer const & channelPutGet,
        epics::pvData::PVStructurePtr const & putPVStructure,
        epics::pvData::BitSet::shared_pointer const & putBitSet)
    {
        easyPutGet->getPutDone(status,channelPutGet,putPVStructure,putBitSet);
    }
    void getGetDone(
        const epics::pvData::Status& status,
        epics::pvAccess::ChannelPutGet::shared_pointer const & channelPutGet,
        epics::pvData::PVStructurePtr const & getPVStructure,
        epics::pvData::BitSet::shared_pointer const & getBitSet)
    {
        easyPutGet->getGetDone(status,channelPutGet,getPVStructure,getBitSet);
    }
};

EasyPutGet::EasyPutGet(
        EasyPVAPtr const &pva,
        EasyChannelPtr const & easyChannel,
        Channel::shared_pointer const & channel,
        PVStructurePtr const &pvRequest)
: easyPVA(pva),
  easyChannel(easyChannel),
  channel(channel),
  pvRequest(pvRequest),
  isDestroyed(false),
  connectState(connectIdle),
  putGetState(putGetIdle)
{
}

EasyPutGet::~EasyPutGet()
{
    destroy();
}

void EasyPutGet::checkPutGetState()
{
    if(isDestroyed) throw std::runtime_error("easyPutGet was destroyed");
    if(connectState==connectIdle){
        connect();
        getPut();
    }
}

// from ChannelPutGetRequester
string EasyPutGet::getRequesterName()
{
     EasyPVAPtr yyy = easyPVA.lock();
     if(!yyy) throw std::runtime_error("easyPVA was destroyed");
     return yyy->getRequesterName();
}

void EasyPutGet::message(string const & message,MessageType messageType)
{
    if(isDestroyed) throw std::runtime_error("easyPutGet was destroyed");
    EasyPVAPtr yyy = easyPVA.lock();
    if(!yyy) throw std::runtime_error("easyPVA was destroyed");
    yyy->message(message, messageType);
}

void EasyPutGet::channelPutGetConnect(
    const Status& status,
    ChannelPutGet::shared_pointer const & channelPutGet,
    StructureConstPtr const & putStructure,
    StructureConstPtr const & getStructure)
{
    if(isDestroyed) throw std::runtime_error("easyPutGet was destroyed");
    channelPutGetConnectStatus = status;
    this->channelPutGet = channelPutGet;
    if(status.isOK()) {
        easyPutData = EasyPutData::create(putStructure);
        easyPutData->setMessagePrefix(easyChannel.lock()->getChannelName());
        easyGetData = EasyGetData::create(getStructure);
        easyGetData->setMessagePrefix(easyChannel.lock()->getChannelName());
    }
    waitForConnect.signal();
    
}

void EasyPutGet::putGetDone(
        const Status& status,
        ChannelPutGet::shared_pointer const & channelPutGet,
        PVStructurePtr const & getPVStructure,
        BitSetPtr const & getBitSet)
{
    if(isDestroyed) throw std::runtime_error("easyPutGet was destroyed");
    channelPutGetStatus = status;
    if(status.isOK()) {
        easyGetData->setData(getPVStructure,getBitSet);
    }
    waitForPutGet.signal();
}

void EasyPutGet::getPutDone(
    const Status& status,
    ChannelPutGet::shared_pointer const & channelPutGet,
    PVStructurePtr const & putPVStructure,
    BitSetPtr const & putBitSet)
{
    if(isDestroyed) throw std::runtime_error("easyPutGet was destroyed");
    channelGetPutGetStatus = status;
    if(status.isOK()) {
        PVStructurePtr pvs = easyPutData->getPVStructure();
        pvs->copyUnchecked(*putPVStructure,*putBitSet);
        BitSetPtr bs = easyPutData->getBitSet();
        bs->clear();
        *bs |= *putBitSet;
    }
    waitForPutGet.signal();
}

void EasyPutGet::getGetDone(
        const Status& status,
        ChannelPutGet::shared_pointer const & channelPutGet,
        PVStructurePtr const & getPVStructure,
        BitSetPtr const & getBitSet)
{
    if(isDestroyed) throw std::runtime_error("easyPutGet was destroyed");
    channelPutGetStatus = status;
    if(status.isOK()) {
        easyGetData->setData(getPVStructure,getBitSet);
    }
    waitForPutGet.signal();
}



// from EasyPutGet
void EasyPutGet::destroy()
{
    {
        Lock xx(mutex);
        if(isDestroyed) return;
        isDestroyed = true;
    }
    if(channelPutGet) channelPutGet->destroy();
    channelPutGet.reset();
}

void EasyPutGet::connect()
{
    if(isDestroyed) throw std::runtime_error("easyPutGet was destroyed");
    issueConnect();
    Status status = waitConnect();
    if(status.isOK()) return;
    stringstream ss;
    ss << "channel " << channel->getChannelName() << " EasyPutGet::connect " << status.getMessage();
    throw std::runtime_error(ss.str());
}

void EasyPutGet::issueConnect()
{
    if(isDestroyed) throw std::runtime_error("easyPutGet was destroyed");
    if(connectState!=connectIdle) {
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " easyPutGet already connected ";
        throw std::runtime_error(ss.str());
    }
    putGetRequester = ChannelPutGetRequester::shared_pointer(new ChannelPutGetRequesterImpl(this));
    connectState = connectActive;
    channelPutGet = channel->createChannelPutGet(putGetRequester,pvRequest);
}

Status EasyPutGet::waitConnect()
{
    if(isDestroyed) throw std::runtime_error("easyPutGet was destroyed");
    if(connectState!=connectActive) {
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " easyPutGet illegal connect state ";
        throw std::runtime_error(ss.str());
    }
    waitForConnect.wait();
    if(channelPutGetConnectStatus.isOK()) {
        connectState = connected;
        return Status::Ok;
    }
    connectState = connectIdle;
    return Status(Status::STATUSTYPE_ERROR,channelPutGetConnectStatus.getMessage());
}


void EasyPutGet::putGet()
{
    if(isDestroyed) throw std::runtime_error("easyPutGet was destroyed");
    issuePutGet();
    Status status = waitPutGet();
    if(status.isOK()) return;
    stringstream ss;
    ss << "channel " << channel->getChannelName() << " EasyPutGet::putGet " << status.getMessage();
    throw std::runtime_error(ss.str());
}

void EasyPutGet::issuePutGet()
{
    if(isDestroyed) throw std::runtime_error("easyPutGet was destroyed");
    if(connectState==connectIdle) connect();
    if(putGetState!=putGetIdle) {
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " EasyPutGet::issueGet get or put aleady active ";
        throw std::runtime_error(ss.str());
    }
    putGetState = putGetActive;
    channelPutGet->putGet(easyPutData->getPVStructure(),easyPutData->getBitSet());
}


Status EasyPutGet::waitPutGet()
{
    if(isDestroyed) throw std::runtime_error("easyPutGet was destroyed");
    if(putGetState!=putGetActive){
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " EasyPutGet::waitPutGet llegal put state";
        throw std::runtime_error(ss.str());
    }
    waitForPutGet.wait();
    putGetState = putGetIdle;
    if(channelGetPutGetStatus.isOK()) {
        return Status::Ok;
    }
    return Status(Status::STATUSTYPE_ERROR,channelGetPutGetStatus.getMessage());
}

void EasyPutGet::getGet()
{
    if(isDestroyed) throw std::runtime_error("easyPutGet was destroyed");
    issueGetGet();
    Status status = waitGetGet();
    if(status.isOK()) return;
    stringstream ss;
    ss << "channel " << channel->getChannelName() << " EasyPutGet::getGet " << status.getMessage();
    throw std::runtime_error(ss.str());
}

void EasyPutGet::issueGetGet()
{
    if(isDestroyed) throw std::runtime_error("easyPutGet was destroyed");
    if(connectState==connectIdle) connect();
    if(putGetState!=putGetIdle) {
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " EasyPutGet::issueGetGet aleady active ";
        throw std::runtime_error(ss.str());
    }
    putGetState = putGetActive;
    channelPutGet->getGet();
}

Status EasyPutGet::waitGetGet()
{
    if(isDestroyed) throw std::runtime_error("easyPutGet was destroyed");
    if(putGetState!=putGetActive){
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " EasyPutGet::waitGetGet illegal state";
        throw std::runtime_error(ss.str());
    }
    waitForPutGet.wait();
    putGetState = putGetIdle;
    if(channelGetPutGetStatus.isOK()) {
        return Status::Ok;
    }
    return Status(Status::STATUSTYPE_ERROR,channelGetPutGetStatus.getMessage());
}

void EasyPutGet::getPut()
{
    if(isDestroyed) throw std::runtime_error("easyPutGet was destroyed");
    issueGetPut();
    Status status = waitGetPut();
    if(status.isOK()) return;
    stringstream ss;
    ss << "channel " << channel->getChannelName() << " EasyPutGet::getPut " << status.getMessage();
    throw std::runtime_error(ss.str());
}

void EasyPutGet::issueGetPut()
{
    if(isDestroyed) throw std::runtime_error("easyPutGet was destroyed");
    if(connectState==connectIdle) connect();
    if(putGetState!=putGetIdle) {
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " EasyPutGet::issueGetPut aleady active ";
        throw std::runtime_error(ss.str());
    }
    putGetState = putGetActive;
    channelPutGet->getPut();
}

Status EasyPutGet::waitGetPut()
{
    if(isDestroyed) throw std::runtime_error("easyPutGet was destroyed");
    if(putGetState!=putGetActive){
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " EasyPutGet::waitGetPut illegal state";
        throw std::runtime_error(ss.str());
    }
    waitForPutGet.wait();
    putGetState = putGetIdle;
    if(channelGetPutGetStatus.isOK()) {
        return Status::Ok;
    }
    return Status(Status::STATUSTYPE_ERROR,channelGetPutGetStatus.getMessage());
}

EasyGetDataPtr EasyPutGet::getGetData()
{
    checkPutGetState();
    return easyGetData;
}

EasyPutDataPtr EasyPutGet::getPutData()
{
    checkPutGetState();
    return easyPutData;
}

EasyPutGetPtr EasyPutGet::create(
        EasyPVAPtr const &pva,
        EasyChannelPtr const & easyChannel,
        Channel::shared_pointer const & channel,
        PVStructurePtr const &pvRequest)
{
    EasyPutGetPtr epv(new EasyPutGet(pva,easyChannel,channel,pvRequest));
    return epv;
}


}}

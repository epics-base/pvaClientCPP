/* easyPut.cpp */
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

class ChannelPutRequesterImpl : public ChannelPutRequester
{
    EasyPut * easyPut;
public:
    ChannelPutRequesterImpl(EasyPut * easyPut)
    : easyPut(easyPut) {}
    string getRequesterName()
    {return easyPut->getRequesterName();}
    void message(string const & message,MessageType messageType)
    {easyPut->message(message,messageType);}
    void channelPutConnect(
        const Status& status,
        ChannelPut::shared_pointer const & channelPut,
        StructureConstPtr const & structure)
    {easyPut->channelPutConnect(status,channelPut,structure);}
    void getDone(
        const Status& status,
        ChannelPut::shared_pointer const & channelPut,
        PVStructurePtr const & pvStructure,
        BitSetPtr const & bitSet)
    {easyPut->getDone(status,channelPut,pvStructure,bitSet);}
    void putDone(
        const Status& status,
        ChannelPut::shared_pointer const & channelPut)
    {easyPut->putDone(status,channelPut);}
};

EasyPut::EasyPut(
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
  putState(putIdle)
{
}

EasyPut::~EasyPut()
{
    destroy();
}

void EasyPut::checkPutState()
{
    if(isDestroyed) throw std::runtime_error("easyPut was destroyed");
    if(connectState==connectIdle){
          connect();
          get();
    }
}

// from ChannelPutRequester
string EasyPut::getRequesterName()
{
     EasyPVAPtr yyy = easyPVA.lock();
     if(!yyy) throw std::runtime_error("easyPVA was destroyed");
     return yyy->getRequesterName();
}

void EasyPut::message(string const & message,MessageType messageType)
{
    if(isDestroyed) throw std::runtime_error("easyPut was destroyed");
    EasyPVAPtr yyy = easyPVA.lock();
    if(!yyy) throw std::runtime_error("easyPVA was destroyed");
    yyy->message(message, messageType);
}

void EasyPut::channelPutConnect(
    const Status& status,
    ChannelPut::shared_pointer const & channelPut,
    StructureConstPtr const & structure)
{
    if(isDestroyed) throw std::runtime_error("easyPut was destroyed");
    channelPutConnectStatus = status;
    this->channelPut = channelPut;
    if(status.isOK()) {
        easyData = EasyPutData::create(structure);
        easyData->setMessagePrefix(easyChannel.lock()->getChannelName());
    }
    waitForConnect.signal();
    
}

void EasyPut::getDone(
    const Status& status,
    ChannelPut::shared_pointer const & channelPut,
    PVStructurePtr const & pvStructure,
    BitSetPtr const & bitSet)
{
    if(isDestroyed) throw std::runtime_error("easyPut was destroyed");
    channelGetPutStatus = status;
    if(status.isOK()) {
        PVStructurePtr pvs = easyData->getPVStructure();
        pvs->copyUnchecked(*pvStructure,*bitSet);
        BitSetPtr bs = easyData->getBitSet();
        bs->clear();
        *bs |= *bitSet;
    }
    waitForGetPut.signal();
}

void EasyPut::putDone(
    const Status& status,
    ChannelPut::shared_pointer const & channelPut)
{
    if(isDestroyed) throw std::runtime_error("easyPut was destroyed");
    channelGetPutStatus = status;
    waitForGetPut.signal();
}


// from EasyPut
void EasyPut::destroy()
{
    {
        Lock xx(mutex);
        if(isDestroyed) return;
        isDestroyed = true;
    }
    if(channelPut) channelPut->destroy();
    channelPut.reset();
}

void EasyPut::connect()
{
    if(isDestroyed) throw std::runtime_error("easyPut was destroyed");
    issueConnect();
    Status status = waitConnect();
    if(status.isOK()) return;
    stringstream ss;
    ss << "channel " << channel->getChannelName() << " EasyPut::connect " << status.getMessage();
    throw std::runtime_error(ss.str());
}

void EasyPut::issueConnect()
{
    if(isDestroyed) throw std::runtime_error("easyPut was destroyed");
    if(connectState!=connectIdle) {
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " easyPut already connected ";
        throw std::runtime_error(ss.str());
    }
    putRequester = ChannelPutRequester::shared_pointer(new ChannelPutRequesterImpl(this));
    connectState = connectActive;
    channelPut = channel->createChannelPut(putRequester,pvRequest);
}

Status EasyPut::waitConnect()
{
    if(isDestroyed) throw std::runtime_error("easyPut was destroyed");
    if(connectState!=connectActive) {
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " easyPut illegal connect state ";
        throw std::runtime_error(ss.str());
    }
    waitForConnect.wait();
    if(channelPutConnectStatus.isOK()) {
        connectState = connected;
        return Status::Ok;
    }
    connectState = connectIdle;
    return Status(Status::STATUSTYPE_ERROR,channelPutConnectStatus.getMessage());
}

void EasyPut::get()
{
    if(isDestroyed) throw std::runtime_error("easyPut was destroyed");
    issueGet();
    Status status = waitGet();
    if(status.isOK()) return;
    stringstream ss;
    ss << "channel " << channel->getChannelName() << " EasyPut::get " << status.getMessage();
    throw std::runtime_error(ss.str());
}

void EasyPut::issueGet()
{
    if(isDestroyed) throw std::runtime_error("easyPut was destroyed");
    if(connectState==connectIdle) connect();
    if(putState!=putIdle) {
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " EasyPut::issueGet get or put aleady active ";
        throw std::runtime_error(ss.str());
    }
    putState = getActive;
    easyData->getBitSet()->clear();
    channelPut->get();
}

Status EasyPut::waitGet()
{
    if(isDestroyed) throw std::runtime_error("easyPut was destroyed");
    if(putState!=getActive){
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " EasyPut::waitGet llegal put state";
        throw std::runtime_error(ss.str());
    }
    waitForGetPut.wait();
    putState = putIdle;
    if(channelGetPutStatus.isOK()) {
        return Status::Ok;
    }
    return Status(Status::STATUSTYPE_ERROR,channelGetPutStatus.getMessage());
}

void EasyPut::put()
{
    if(isDestroyed) throw std::runtime_error("easyPut was destroyed");
    issuePut();
    Status status = waitPut();
    if(status.isOK()) return;
    stringstream ss;
    ss << "channel " << channel->getChannelName() << " EasyPut::put " << status.getMessage();
    throw std::runtime_error(ss.str());
}

void EasyPut::issuePut()
{
    if(isDestroyed) throw std::runtime_error("easyPut was destroyed");
    if(connectState==connectIdle) connect();
    if(putState!=putIdle) {
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " EasyPut::issueGet get or put aleady active ";
        throw std::runtime_error(ss.str());
    }
    putState = putActive;
    channelPut->put(easyData->getPVStructure(),easyData->getBitSet());
}

Status EasyPut::waitPut()
{
    if(isDestroyed) throw std::runtime_error("easyPut was destroyed");
    if(putState!=putActive){
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " EasyPut::waitPut llegal put state";
        throw std::runtime_error(ss.str());
    }
    waitForGetPut.wait();
    putState = putIdle;
    if(channelGetPutStatus.isOK()) {
        easyData->getBitSet()->clear();
        return Status::Ok;
    }
    return Status(Status::STATUSTYPE_ERROR,channelGetPutStatus.getMessage());
}

EasyPutDataPtr EasyPut::getData()
{
    checkPutState();
    return easyData;
}

EasyPutPtr EasyPut::create(
        EasyPVAPtr const &pva,
        EasyChannelPtr const & easyChannel,
        Channel::shared_pointer const & channel,
        PVStructurePtr const &pvRequest)
{
    EasyPutPtr epv(new EasyPut(pva,easyChannel,channel,pvRequest));
    return epv;
}


}}

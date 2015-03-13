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
#include <pv/easyPVA.h>

using std::tr1::static_pointer_cast;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace std;

namespace epics { namespace easyPVA {


class ChannelGetRequesterImpl : public ChannelGetRequester
{
    EasyGet * easyGet;
public:
    ChannelGetRequesterImpl(EasyGet * easyGet)
    : easyGet(easyGet) {}
    string getRequesterName()
    {return easyGet->getRequesterName();}
    void message(string const & message,MessageType messageType)
    {easyGet->message(message,messageType);}
    void channelGetConnect(
        const Status& status,
        ChannelGet::shared_pointer const & channelGet,
        StructureConstPtr const & structure)
    {easyGet->channelGetConnect(status,channelGet,structure);}
    void getDone(
        const Status& status,
        ChannelGet::shared_pointer const & channelGet,
        PVStructurePtr const & pvStructure,
        BitSetPtr const & bitSet)
    {easyGet->getDone(status,channelGet,pvStructure,bitSet);}
};

EasyGet::EasyGet(
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
  getState(getIdle)
{
}

EasyGet::~EasyGet()
{
    destroy();
}

void EasyGet::checkGetState()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    if(connectState==connectIdle) connect();
    if(getState==getIdle) get();
}

// from ChannelGetRequester
string EasyGet::getRequesterName()
{
     EasyPVAPtr yyy = easyPVA.lock();
     if(!yyy) throw std::runtime_error("easyPVA was destroyed");
     return yyy->getRequesterName();
}

void EasyGet::message(string const & message,MessageType messageType)
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    EasyPVAPtr yyy = easyPVA.lock();
    if(!yyy) throw std::runtime_error("easyPVA was destroyed");
    yyy->message(message, messageType);
}

void EasyGet::channelGetConnect(
    const Status& status,
    ChannelGet::shared_pointer const & channelGet,
    StructureConstPtr const & structure)
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    channelGetConnectStatus = status;
    this->channelGet = channelGet;
    if(status.isOK()) {
        easyData = EasyGetData::create(structure);
        easyData->setMessagePrefix(channel->getChannelName());
    }
    waitForConnect.signal();
    
}

void EasyGet::getDone(
    const Status& status,
    ChannelGet::shared_pointer const & channelGet,
    PVStructurePtr const & pvStructure,
    BitSetPtr const & bitSet)
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    channelGetStatus = status;
    if(status.isOK()) {
        easyData->setData(pvStructure,bitSet);
    }
    waitForGet.signal();
}


// from EasyGet
void EasyGet::destroy()
{
    {
        Lock xx(mutex);
        if(isDestroyed) return;
        isDestroyed = true;
    }
    if(channelGet) channelGet->destroy();
    channelGet.reset();
}

void EasyGet::connect()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    issueConnect();
    Status status = waitConnect();
    if(status.isOK()) return;
    stringstream ss;
    ss << "channel " << channel->getChannelName() << " EasyGet::connect " << status.getMessage();
    throw std::runtime_error(ss.str());
}

void EasyGet::issueConnect()
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

Status EasyGet::waitConnect()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    if(connectState!=connectActive) {
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " easyGet illegal connect state ";
        throw std::runtime_error(ss.str());
    }
    waitForConnect.wait();
    if(channelGetConnectStatus.isOK()){
        connectState = connected;
        return Status::Ok;
    }
    connectState = connectIdle;
    return Status(Status::STATUSTYPE_ERROR,channelGetConnectStatus.getMessage());
}

void EasyGet::get()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    issueGet();
    Status status = waitGet();
    if(status.isOK()) return;
    stringstream ss;
    ss << "channel " << channel->getChannelName() << " EasyGet::get " << status.getMessage();
    throw std::runtime_error(ss.str());
}

void EasyGet::issueGet()
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

Status EasyGet::waitGet()
{
    if(isDestroyed) throw std::runtime_error("easyGet was destroyed");
    if(getState!=getActive){
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " EasyGet::waitGet llegal get state";
        throw std::runtime_error(ss.str());
    }
    waitForGet.wait();
    getState = getIdle;
    if(channelGetStatus.isOK()) {
        return Status::Ok;
    }
    return Status(Status::STATUSTYPE_ERROR,channelGetStatus.getMessage());
}
EasyGetDataPtr EasyGet::getData()
{
    checkGetState();
    return easyData;
}

EasyGetPtr EasyGet::create(
        EasyPVAPtr const &pva,
        EasyChannelPtr const & easyChannel,
        Channel::shared_pointer const & channel,
        PVStructurePtr const &pvRequest)
{
    EasyGetPtr epv(new EasyGet(pva,easyChannel,channel,pvRequest));
    return epv;
}

}}

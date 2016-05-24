/* pvaClientPutGet.cpp */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvData is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
/**
 * @author mrk
 * @date 2015.02
 */
#include <pv/event.h>

#define epicsExportSharedSymbols

#include <pv/pvaClient.h>

using std::tr1::static_pointer_cast;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace std;

namespace epics { namespace pvaClient {

class ChannelPutGetRequesterImpl : public ChannelPutGetRequester
{
    PvaClientPutGet::weak_pointer pvaClientPutGet;
    PvaClient::weak_pointer pvaClient;
public:
    ChannelPutGetRequesterImpl(
        PvaClientPutGetPtr const & pvaClientPutGet,
        PvaClientPtr const &pvaClient)
    : pvaClientPutGet(pvaClientPutGet),
      pvaClient(pvaClient)
    {}
    virtual ~ChannelPutGetRequesterImpl() {
        if(PvaClient::getDebug()) std::cout << "~ChannelPutGetRequesterImpl" << std::endl;
    }

    virtual std::string getRequesterName() {
        PvaClientPutGetPtr clientPutGet(pvaClientPutGet.lock());
        if(!clientPutGet) return string("clientPutGet is null");
        return clientPutGet->getRequesterName();
    }

    virtual void message(std::string const & message, epics::pvData::MessageType messageType) {
        PvaClientPutGetPtr clientPutGet(pvaClientPutGet.lock());
        if(!clientPutGet) return;
        clientPutGet->message(message,messageType);
    }

    virtual void channelPutGetConnect(
        const Status& status,
        ChannelPutGet::shared_pointer const & channelPutGet,
        Structure::const_shared_pointer const & putStructure,
        Structure::const_shared_pointer const & getStructure)
    {
        PvaClientPutGetPtr clientPutGet(pvaClientPutGet.lock());
        if(!clientPutGet) return;
        clientPutGet->channelPutGetConnect(status,channelPutGet,putStructure,getStructure);  
    }

    virtual void putGetDone(
        const Status& status,
        ChannelPutGet::shared_pointer const & channelPutGet,
        PVStructurePtr const & getPVStructure,
        BitSet::shared_pointer const & getBitSet)
    {
        PvaClientPutGetPtr clientPutGet(pvaClientPutGet.lock());
        if(!clientPutGet) return;
        clientPutGet->putGetDone(status,channelPutGet,getPVStructure,getBitSet);
    }

    virtual void getPutDone(
        const Status& status,
        ChannelPutGet::shared_pointer const & channelPutGet,
        PVStructurePtr const & putPVStructure,
        BitSet::shared_pointer const & putBitSet)
    {
        PvaClientPutGetPtr clientPutGet(pvaClientPutGet.lock());
        if(!clientPutGet) return;
        clientPutGet->getPutDone(status,channelPutGet,putPVStructure,putBitSet);
    }


    virtual void getGetDone(
        const Status& status,
        ChannelPutGet::shared_pointer const & channelPutGet,
        PVStructurePtr const & getPVStructure,
        BitSet::shared_pointer const & getBitSet)
    {
        PvaClientPutGetPtr clientPutGet(pvaClientPutGet.lock());
        if(!clientPutGet) return;
        clientPutGet->getGetDone(status,channelPutGet,getPVStructure,getBitSet);
    }
};

PvaClientPutGetPtr PvaClientPutGet::create(
        PvaClientPtr const &pvaClient,
        Channel::shared_pointer const & channel,
        PVStructurePtr const &pvRequest)
{
    PvaClientPutGetPtr epv(new PvaClientPutGet(pvaClient,channel,pvRequest));
    epv->channelPutGetRequester = ChannelPutGetRequesterImplPtr(
        new ChannelPutGetRequesterImpl(epv,pvaClient));
    return epv;
}

PvaClientPutGet::PvaClientPutGet(
        PvaClientPtr const &pvaClient,
        Channel::shared_pointer const & channel,
        PVStructurePtr const &pvRequest)
: pvaClient(pvaClient),
  channel(channel),
  pvRequest(pvRequest),
  isDestroyed(false),
  connectState(connectIdle),
  putGetState(putGetIdle)
{
    if(PvaClient::getDebug()) cout<< "PvaClientPutGet::PvaClientPutGet()\n";
}

PvaClientPutGet::~PvaClientPutGet()
{
    if(PvaClient::getDebug()) cout<< "PvaClientPutGet::~PvaClientPutGet()\n";
    {
        Lock xx(mutex);
        if(isDestroyed) throw std::runtime_error("pvaClientPutGet was destroyed");
        isDestroyed = true;
    }
    channelPutGet->destroy();
}

void PvaClientPutGet::checkPutGetState()
{
    if(connectState==connectIdle){
        connect();
        getPut();
    }
}

string PvaClientPutGet::getRequesterName()
{
     PvaClientPtr yyy = pvaClient.lock();
     if(!yyy) throw std::runtime_error("pvaClient was destroyed");
     return yyy->getRequesterName();
}

void PvaClientPutGet::message(string const & message,MessageType messageType)
{
    PvaClientPtr yyy = pvaClient.lock();
    if(!yyy) throw std::runtime_error("pvaClient was destroyed");
    yyy->message(message, messageType);
}

void PvaClientPutGet::channelPutGetConnect(
    const Status& status,
    ChannelPutGet::shared_pointer const & channelPutGet,
    StructureConstPtr const & putStructure,
    StructureConstPtr const & getStructure)
{
    channelPutGetConnectStatus = status;
    this->channelPutGet = channelPutGet;
    if(status.isOK()) {
        pvaClientPutData = PvaClientPutData::create(putStructure);
        pvaClientPutData->setMessagePrefix(channel->getChannelName());
        pvaClientGetData = PvaClientGetData::create(getStructure);
        pvaClientGetData->setMessagePrefix(channel->getChannelName());
    }
    waitForConnect.signal();
    
}

void PvaClientPutGet::putGetDone(
        const Status& status,
        ChannelPutGet::shared_pointer const & channelPutGet,
        PVStructurePtr const & getPVStructure,
        BitSetPtr const & getChangedBitSet)
{
    channelPutGetStatus = status;
    if(status.isOK()) {
        pvaClientGetData->setData(getPVStructure,getChangedBitSet);
    }
    waitForPutGet.signal();
}

void PvaClientPutGet::getPutDone(
    const Status& status,
    ChannelPutGet::shared_pointer const & channelPutGet,
    PVStructurePtr const & putPVStructure,
    BitSetPtr const & putBitSet)
{
    channelPutGetStatus = status;
    if(status.isOK()) {
        PVStructurePtr pvs = pvaClientPutData->getPVStructure();
        pvs->copyUnchecked(*putPVStructure,*putBitSet);
        BitSetPtr bs = pvaClientPutData->getChangedBitSet();
        bs->clear();
        *bs |= *putBitSet;
    }
    waitForPutGet.signal();
}

void PvaClientPutGet::getGetDone(
        const Status& status,
        ChannelPutGet::shared_pointer const & channelPutGet,
        PVStructurePtr const & getPVStructure,
        BitSetPtr const & getChangedBitSet)
{
    channelPutGetStatus = status;
    if(status.isOK()) {
        pvaClientGetData->setData(getPVStructure,getChangedBitSet);
    }
    waitForPutGet.signal();
}

void PvaClientPutGet::connect()
{
    issueConnect();
    Status status = waitConnect();
    if(status.isOK()) return;
    string message = string("channel ") + channel->getChannelName()
        + " PvaClientPutGet::connect " + status.getMessage();
    throw std::runtime_error(message);
}

void PvaClientPutGet::issueConnect()
{
    if(connectState!=connectIdle) {
        string message = string("channel ") + channel->getChannelName()
            + " pvaClientPutGet already connected ";
        throw std::runtime_error(message);
    }
    connectState = connectActive;
    channelPutGet = channel->createChannelPutGet(channelPutGetRequester,pvRequest);
}

Status PvaClientPutGet::waitConnect()
{
    if(connectState!=connectActive) {
        string message = string("channel ") + channel->getChannelName()
            + " pvaClientPutGet illegal connect state ";
        throw std::runtime_error(message);
    }
    waitForConnect.wait();
    connectState = channelPutGetConnectStatus.isOK() ? connected : connectIdle;
    return channelPutGetConnectStatus;
}


void PvaClientPutGet::putGet()
{
    issuePutGet();
    Status status = waitPutGet();
    if(status.isOK()) return;
    string message = string("channel ") + channel->getChannelName()
        + " PvaClientPutGet::putGet " + status.getMessage();
    throw std::runtime_error(message);
}

void PvaClientPutGet::issuePutGet()
{
    if(connectState==connectIdle) connect();
    if(putGetState!=putGetIdle) {
        string message = string("channel ") + channel->getChannelName()
            + " PvaClientPutGet::issueGet get or put aleady active ";
        throw std::runtime_error(message);
    }
    putGetState = putGetActive;
    channelPutGet->putGet(pvaClientPutData->getPVStructure(),pvaClientPutData->getChangedBitSet());
}


Status PvaClientPutGet::waitPutGet()
{
    if(putGetState!=putGetActive){
        string message = string("channel ") + channel->getChannelName()
            + " PvaClientPutGet::waitPutGet llegal put state";
        throw std::runtime_error(message);
    }
    waitForPutGet.wait();
    putGetState = putGetIdle;
    return channelPutGetStatus;
}

void PvaClientPutGet::getGet()
{
    issueGetGet();
    Status status = waitGetGet();
    if(status.isOK()) return;
    string message = string("channel ") + channel->getChannelName()
        + " PvaClientPutGet::getGet " + status.getMessage();
    throw std::runtime_error(message);
}

void PvaClientPutGet::issueGetGet()
{
    if(connectState==connectIdle) connect();
    if(putGetState!=putGetIdle) {
        string message = string("channel ") + channel->getChannelName()
            + " PvaClientPutGet::issueGetGet aleady active ";
        throw std::runtime_error(message);
    }
    putGetState = putGetActive;
    channelPutGet->getGet();
}

Status PvaClientPutGet::waitGetGet()
{
    if(putGetState!=putGetActive){
        string message = string("channel ") + channel->getChannelName()
            + " PvaClientPutGet::waitGetGet illegal state";
        throw std::runtime_error(message);
    }
    waitForPutGet.wait();
    putGetState = putGetIdle;
    return channelPutGetStatus;
}

void PvaClientPutGet::getPut()
{
    issueGetPut();
    Status status = waitGetPut();
    if(status.isOK()) return;
    string message = string("channel ") + channel->getChannelName()
        + " PvaClientPutGet::getPut " + status.getMessage();
    throw std::runtime_error(message);
}

void PvaClientPutGet::issueGetPut()
{
    if(connectState==connectIdle) connect();
    if(putGetState!=putGetIdle) {
        string message = string("channel ") + channel->getChannelName()
            + " PvaClientPutGet::issueGetPut aleady active ";
        throw std::runtime_error(message);
    }
    putGetState = putGetActive;
    channelPutGet->getPut();
}

Status PvaClientPutGet::waitGetPut()
{
    if(putGetState!=putGetActive){
         string message = string("channel ") + channel->getChannelName()
            + " PvaClientPutGet::waitGetPut illegal state";
        throw std::runtime_error(message);
    }
    waitForPutGet.wait();
    putGetState = putGetIdle;
    return channelPutGetStatus;
}

PvaClientGetDataPtr PvaClientPutGet::getGetData()
{
    checkPutGetState();
    return pvaClientGetData;
}

PvaClientPutDataPtr PvaClientPutGet::getPutData()
{
    checkPutGetState();
    return pvaClientPutData;
}

}}

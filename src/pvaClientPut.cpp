/* pvaClientPut.cpp */
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

PvaClientPut::PvaClientPut(
        PvaClientPtr const &pvaClient,
        Channel::shared_pointer const & channel,
        PVStructurePtr const &pvRequest)
: pvaClient(pvaClient),
  channel(channel),
  pvRequest(pvRequest),
  isDestroyed(false),
  connectState(connectIdle),
  putState(putIdle)
{
    if(PvaClient::getDebug()) cout<< "PvaClientPut::PvaClientPut()\n";
}

PvaClientPut::~PvaClientPut()
{
    if(PvaClient::getDebug()) cout<< "PvaClientPut::~PvaClientPut()\n";
    {
        Lock xx(mutex);
        if(isDestroyed) {
             cerr<< "Why was PvaClientPut::~PvaClientPut() called more then once????\n";
             return;
        }
        isDestroyed = true;
    }
    if(channelPut) channelPut->destroy();
}

void PvaClientPut::checkPutState()
{
    if(isDestroyed) throw std::runtime_error("pvaClientPut was destroyed");
    if(connectState==connectIdle){
          connect();
          get();
    }
}

// from ChannelPutRequester
string PvaClientPut::getRequesterName()
{
     PvaClientPtr yyy = pvaClient.lock();
     if(!yyy) throw std::runtime_error("pvaClient was destroyed");
     return yyy->getRequesterName();
}

void PvaClientPut::message(string const & message,MessageType messageType)
{
    if(isDestroyed) throw std::runtime_error("pvaClientPut was destroyed");
    PvaClientPtr yyy = pvaClient.lock();
    if(!yyy) throw std::runtime_error("pvaClient was destroyed");
    yyy->message(message, messageType);
}

void PvaClientPut::channelPutConnect(
    const Status& status,
    ChannelPut::shared_pointer const & channelPut,
    StructureConstPtr const & structure)
{
    if(isDestroyed) throw std::runtime_error("pvaClientPut was destroyed");
    channelPutConnectStatus = status;
    this->channelPut = channelPut;
    if(status.isOK()) {
        pvaClientData = PvaClientPutData::create(structure);
        pvaClientData->setMessagePrefix(channel->getChannelName());
    }
    waitForConnect.signal();
    
}

void PvaClientPut::getDone(
    const Status& status,
    ChannelPut::shared_pointer const & channelPut,
    PVStructurePtr const & pvStructure,
    BitSetPtr const & bitSet)
{
    if(isDestroyed) throw std::runtime_error("pvaClientPut was destroyed");
    channelGetPutStatus = status;
    connectState = connected;
    if(status.isOK()) {
        PVStructurePtr pvs = pvaClientData->getPVStructure();
        pvs->copyUnchecked(*pvStructure,*bitSet);
        BitSetPtr bs = pvaClientData->getChangedBitSet();
        bs->clear();
        *bs |= *bitSet;
    }
    waitForGetPut.signal();
}

void PvaClientPut::putDone(
    const Status& status,
    ChannelPut::shared_pointer const & channelPut)
{
    if(isDestroyed) throw std::runtime_error("pvaClientPut was destroyed");
    channelGetPutStatus = status;
    waitForGetPut.signal();
}

void PvaClientPut::connect()
{
    if(isDestroyed) throw std::runtime_error("pvaClientPut was destroyed");
    issueConnect();
    Status status = waitConnect();
    if(status.isOK()) return;
    string message = string("channel ") 
        + channel->getChannelName()
        + " PvaClientPut::connect "
        + status.getMessage();
    throw std::runtime_error(message);
}

void PvaClientPut::issueConnect()
{
    if(isDestroyed) throw std::runtime_error("pvaClientPut was destroyed");
    if(connectState!=connectIdle) {
        string message = string("channel ") + channel->getChannelName()
            + " pvaClientPut already connected ";
        throw std::runtime_error(message);
    }
    ChannelPutRequester::shared_pointer putRequester(shared_from_this());
    connectState = connectActive;
    channelPut = channel->createChannelPut(putRequester,pvRequest);
}

Status PvaClientPut::waitConnect()
{
    if(isDestroyed) throw std::runtime_error("pvaClientPut was destroyed");
    if(connectState==connected) {
         if(!channelPutConnectStatus.isOK()) connectState = connectIdle;
         return channelPutConnectStatus;
    }
    if(connectState!=connectActive) {
        string message = string("channel ") + channel->getChannelName()
            + " pvaClientPut illegal connect state ";
        throw std::runtime_error(message);
    }
    waitForConnect.wait();
    if(!channelPutConnectStatus.isOK()) connectState = connectIdle;
    return channelPutConnectStatus;
}

void PvaClientPut::get()
{
    if(isDestroyed) throw std::runtime_error("pvaClientPut was destroyed");
    issueGet();
    Status status = waitGet();
    if(status.isOK()) return;
    string message = string("channel ") 
        + channel->getChannelName()
        + " PvaClientPut::get " 
        + status.getMessage();
    throw std::runtime_error(message);
}

void PvaClientPut::issueGet()
{
    if(isDestroyed) throw std::runtime_error("pvaClientPut was destroyed");
    if(connectState==connectIdle) connect();
    if(putState!=putIdle) {
        string message = string("channel ")
            + channel->getChannelName()
            +  "PvaClientPut::issueGet get or put aleady active ";
        throw std::runtime_error(message);
    }
    putState = getActive;
    channelPut->get();
}

Status PvaClientPut::waitGet()
{
    if(isDestroyed) throw std::runtime_error("pvaClientPut was destroyed");
    if(putState!=getActive){
        string message = string("channel ") + channel->getChannelName()
            +  " PvaClientPut::waitGet illegal put state";
        throw std::runtime_error(message);
    }
    waitForGetPut.wait();
    putState = putIdle;
    return channelGetPutStatus;
}

void PvaClientPut::put()
{
    if(isDestroyed) throw std::runtime_error("pvaClientPut was destroyed");
    issuePut();
    Status status = waitPut();
    if(status.isOK()) return;
    string message = string("channel ")
        + channel->getChannelName()
        + " PvaClientPut::put "
        + status.getMessage();
    throw std::runtime_error(message);
}

void PvaClientPut::issuePut()
{
    if(isDestroyed) throw std::runtime_error("pvaClientPut was destroyed");
    if(connectState==connectIdle) connect();
    if(putState!=putIdle) {
         string message = string("channel ") + channel->getChannelName()
            +  "PvaClientPut::issueGet get or put aleady active ";
         throw std::runtime_error(message);
    }
    putState = putActive;
    channelPut->put(pvaClientData->getPVStructure(),pvaClientData->getChangedBitSet());
}

Status PvaClientPut::waitPut()
{
    if(isDestroyed) throw std::runtime_error("pvaClientPut was destroyed");
    if(putState!=putActive){
        string message = string("channel ") + channel->getChannelName()
            +  " PvaClientPut::waitPut illegal put state";
        throw std::runtime_error(message);
    }
    waitForGetPut.wait();
    putState = putIdle;
    if(channelGetPutStatus.isOK()) pvaClientData->getChangedBitSet()->clear();
    return channelGetPutStatus;
}

PvaClientPutDataPtr PvaClientPut::getData()
{
    checkPutState();
    return pvaClientData;
}

PvaClientPutPtr PvaClientPut::create(
        PvaClientPtr const &pvaClient,
        Channel::shared_pointer const & channel,
        PVStructurePtr const &pvRequest)
{
    PvaClientPutPtr epv(new PvaClientPut(pvaClient,channel,pvRequest));
    return epv;
}


}}

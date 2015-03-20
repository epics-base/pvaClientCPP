/* easyMultiChannel.cpp */
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

#include <map>
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



EasyMultiChannel::EasyMultiChannel(
    EasyPVAPtr const &easyPVA,
    PVStringArrayPtr const & channelName,
    string const & providerName)
: easyPVA(easyPVA),
  channelName(channelName),
  providerName(providerName),
  numChannel(channelName->getLength()),
  isConnected(getPVDataCreate()->createPVScalarArray<PVBooleanArray>()),
  isDestroyed(false)
{
}

EasyMultiChannel::~EasyMultiChannel()
{
    destroy();
}

void EasyMultiChannel::destroy()
{
    {
        Lock xx(mutex);
        if(isDestroyed) return;
        isDestroyed = true;
    }
    easyChannelArray.reset();
}

PVStringArrayPtr EasyMultiChannel::getChannelNames()
{
    if(isDestroyed) throw std::runtime_error("easyMultiChannel was destroyed");
    return channelName;
}

Status EasyMultiChannel::connect(double timeout,size_t maxNotConnected)
{
    if(isDestroyed) throw std::runtime_error("easyMultiChannel was destroyed");
    if(easyChannelArray) throw std::runtime_error("easyMultiChannel already connected");
    EasyPVAPtr easy = easyPVA.lock();
    if(!easy) return Status(Status::STATUSTYPE_ERROR,"easyPVA is gone");
    shared_vector<EasyChannelPtr> easyChannel(numChannel,EasyChannelPtr());
    PVStringArray::const_svector channelNames = channelName->view();
    shared_vector<boolean> isConnected(numChannel,false);
    for(size_t i=0; i< numChannel; ++i) {
        easyChannel[i] = easy->createChannel(channelNames[i],providerName);
        easyChannel[i]->issueConnect();
    }
    Status returnStatus = Status::Ok;
    Status status = Status::Ok;
    size_t numBad = 0;
    for(size_t i=0; i< numChannel; ++i) {
	if(numBad==0) {
            status = easyChannel[i]->waitConnect(timeout);
        } else {
            status = easyChannel[i]->waitConnect(.001);
        }
        if(status.isOK()) {
            ++numConnected;
            isConnected[i] = true;
            continue;
        }
        if(returnStatus.isOK()) returnStatus = status;
        ++numBad;
        if(numBad>maxNotConnected) break;
    }
    easyChannelArray = EasyChannelArrayPtr(new EasyChannelArray(freeze(easyChannel)));
    this->isConnected->replace(freeze(isConnected));
    return numBad>maxNotConnected ? returnStatus : Status::Ok;
}


bool EasyMultiChannel::allConnected()
{
    if(isDestroyed) throw std::runtime_error("easyMultiChannel was destroyed");
    if(!easyChannelArray) throw std::runtime_error("easyMultiChannel not connected");
    if(numConnected==numChannel) return true;
    return (numConnected==numChannel) ? true : false;
}

bool EasyMultiChannel::connectionChange()
{
    if(isDestroyed) throw std::runtime_error("easyMultiChannel was destroyed");
    if(!easyChannelArray) throw std::runtime_error("easyMultiChannel not connected");
    if(numConnected==numChannel) return true;
    PVBooleanArray::const_svector isConnected = this->isConnected->view();
    shared_vector<const EasyChannelPtr> channels = *easyChannelArray.get();
    for(size_t i=0; i<numChannel; ++i) {
         const EasyChannelPtr easyChannel = channels[i];
         Channel::shared_pointer channel = easyChannel->getChannel();
         Channel::ConnectionState stateNow = channel->getConnectionState();
         bool connectedNow = stateNow==Channel::CONNECTED ? true : false;
         if(connectedNow!=isConnected[i]) return true;
    }
    return false;
}

PVBooleanArrayPtr EasyMultiChannel::getIsConnected()
{
    if(isDestroyed) throw std::runtime_error("easyMultiChannel was destroyed");
    if(!easyChannelArray) throw std::runtime_error("easyMultiChannel not connected");
    if(!connectionChange()) return isConnected;
    shared_vector<boolean> isConnected(numChannel,false);
    shared_vector<const EasyChannelPtr> channels = *easyChannelArray.get();
    for(size_t i=0; i<numChannel; ++i) {
         const EasyChannelPtr easyChannel = channels[i];
         Channel::shared_pointer channel = easyChannel->getChannel();
         Channel::ConnectionState stateNow = channel->getConnectionState();
         if(stateNow==Channel::CONNECTED) isConnected[i] = true;
    }
    this->isConnected->replace(freeze(isConnected));
    return this->isConnected;
}

EasyChannelArrayWPtr EasyMultiChannel::getEasyChannelArray()
{
    if(isDestroyed) throw std::runtime_error("easyMultiChannel was destroyed");
    if(!easyChannelArray) throw std::runtime_error("easyMultiChannel not connected");
    return easyChannelArray;
}

EasyPVA::weak_pointer EasyMultiChannel::getEasyPVA()
{
    if(isDestroyed) throw std::runtime_error("easyMultiChannel was destroyed");
    return easyPVA;
}

EasyMultiChannelPtr EasyMultiChannel::create(
   EasyPVAPtr const &easyPVA,
   PVStringArrayPtr const & channelNames,
   string const & providerName)
{
    EasyMultiChannelPtr channel(new EasyMultiChannel(easyPVA,channelNames,providerName));
    return channel;
}

}}

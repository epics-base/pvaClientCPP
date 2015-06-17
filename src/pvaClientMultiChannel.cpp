/* pvaClientMultiChannel.cpp */
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
#include <pv/pvaClient.h>
#include <pv/createRequest.h>


using std::tr1::static_pointer_cast;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace std;

namespace epics { namespace pvaClient {



PvaClientMultiChannel::PvaClientMultiChannel(
    PvaClientPtr const &pvaClient,
    PVStringArrayPtr const & channelName,
    string const & providerName)
: pvaClient(pvaClient),
  channelName(channelName),
  providerName(providerName),
  numChannel(channelName->getLength()),
  isConnected(getPVDataCreate()->createPVScalarArray<PVBooleanArray>()),
  isDestroyed(false)
{
}

PvaClientMultiChannel::~PvaClientMultiChannel()
{
    destroy();
}

void PvaClientMultiChannel::destroy()
{
    {
        Lock xx(mutex);
        if(isDestroyed) return;
        isDestroyed = true;
    }
    pvaClientChannelArray.reset();
}

PVStringArrayPtr PvaClientMultiChannel::getChannelNames()
{
    if(isDestroyed) throw std::runtime_error("pvaClientMultiChannel was destroyed");
    return channelName;
}

Status PvaClientMultiChannel::connect(double timeout,size_t maxNotConnected)
{
    if(isDestroyed) throw std::runtime_error("pvaClientMultiChannel was destroyed");
    if(pvaClientChannelArray) throw std::runtime_error("pvaClientMultiChannel already connected");
    PvaClientPtr pvaClient = this->pvaClient.lock();
    if(!pvaClient) return Status(Status::STATUSTYPE_ERROR,"pvaClient is gone");
    shared_vector<PvaClientChannelPtr> pvaClientChannel(numChannel,PvaClientChannelPtr());
    PVStringArray::const_svector channelNames = channelName->view();
    shared_vector<boolean> isConnected(numChannel,false);
    for(size_t i=0; i< numChannel; ++i) {
        pvaClientChannel[i] = pvaClient->createChannel(channelNames[i],providerName);
        pvaClientChannel[i]->issueConnect();
    }
    Status returnStatus = Status::Ok;
    Status status = Status::Ok;
    size_t numBad = 0;
    for(size_t i=0; i< numChannel; ++i) {
	if(numBad==0) {
            status = pvaClientChannel[i]->waitConnect(timeout);
        } else {
            status = pvaClientChannel[i]->waitConnect(.001);
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
    pvaClientChannelArray = PvaClientChannelArrayPtr(new PvaClientChannelArray(freeze(pvaClientChannel)));
    this->isConnected->replace(freeze(isConnected));
    return numBad>maxNotConnected ? returnStatus : Status::Ok;
}


bool PvaClientMultiChannel::allConnected()
{
    if(isDestroyed) throw std::runtime_error("pvaClientMultiChannel was destroyed");
    if(!pvaClientChannelArray) throw std::runtime_error("pvaClientMultiChannel not connected");
    if(numConnected==numChannel) return true;
    return (numConnected==numChannel) ? true : false;
}

bool PvaClientMultiChannel::connectionChange()
{
    if(isDestroyed) throw std::runtime_error("pvaClientMultiChannel was destroyed");
    if(!pvaClientChannelArray) throw std::runtime_error("pvaClientMultiChannel not connected");
    if(numConnected==numChannel) return true;
    PVBooleanArray::const_svector isConnected = this->isConnected->view();
    shared_vector<const PvaClientChannelPtr> channels = *pvaClientChannelArray.get();
    for(size_t i=0; i<numChannel; ++i) {
         const PvaClientChannelPtr pvaClientChannel = channels[i];
         Channel::shared_pointer channel = pvaClientChannel->getChannel();
         Channel::ConnectionState stateNow = channel->getConnectionState();
         bool connectedNow = stateNow==Channel::CONNECTED ? true : false;
         if(connectedNow!=isConnected[i]) return true;
    }
    return false;
}

PVBooleanArrayPtr PvaClientMultiChannel::getIsConnected()
{
    if(isDestroyed) throw std::runtime_error("pvaClientMultiChannel was destroyed");
    if(!pvaClientChannelArray) throw std::runtime_error("pvaClientMultiChannel not connected");
    if(!connectionChange()) return isConnected;
    shared_vector<boolean> isConnected(numChannel,false);
    shared_vector<const PvaClientChannelPtr> channels = *pvaClientChannelArray.get();
    for(size_t i=0; i<numChannel; ++i) {
         const PvaClientChannelPtr pvaClientChannel = channels[i];
         Channel::shared_pointer channel = pvaClientChannel->getChannel();
         Channel::ConnectionState stateNow = channel->getConnectionState();
         if(stateNow==Channel::CONNECTED) isConnected[i] = true;
    }
    this->isConnected->replace(freeze(isConnected));
    return this->isConnected;
}

PvaClientChannelArrayWPtr PvaClientMultiChannel::getPvaClientChannelArray()
{
    if(isDestroyed) throw std::runtime_error("pvaClientMultiChannel was destroyed");
    if(!pvaClientChannelArray) throw std::runtime_error("pvaClientMultiChannel not connected");
    return pvaClientChannelArray;
}

PvaClient::weak_pointer PvaClientMultiChannel::getPvaClient()
{
    if(isDestroyed) throw std::runtime_error("pvaClientMultiChannel was destroyed");
    return pvaClient;
}

PvaClientMultiChannelPtr PvaClientMultiChannel::create(
   PvaClientPtr const &pvaClient,
   PVStringArrayPtr const & channelNames,
   string const & providerName)
{
    PvaClientMultiChannelPtr channel(new PvaClientMultiChannel(pvaClient,channelNames,providerName));
    return channel;
}

}}

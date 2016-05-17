/* pvaClientChannel.cpp */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvData is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
/**
 * @author mrk
 * @date 2015.02
 */

#include <map>
#include <pv/event.h>
#include <pv/lock.h>
#include <pv/createRequest.h>

#define epicsExportSharedSymbols

#include <pv/pvaClient.h>

using std::tr1::static_pointer_cast;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace std;

namespace epics { namespace pvaClient {


class PvaClientGetCache
{
public:
    PvaClientGetCache(){}
    ~PvaClientGetCache()
    {
        pvaClientGetMap.clear();
    }
    PvaClientGetPtr getGet(string const & request);
    void addGet(string const & request,PvaClientGetPtr const & pvaClientGet);
    void showCache();
    size_t cacheSize();
private:
    map<string,PvaClientGetPtr> pvaClientGetMap;
};

PvaClientGetPtr PvaClientGetCache::getGet(string const & request)
{
    map<string,PvaClientGetPtr>::iterator iter = pvaClientGetMap.find(request);
    if(iter!=pvaClientGetMap.end()) return iter->second;
    return PvaClientGetPtr();
}

void PvaClientGetCache::addGet(string const & request,PvaClientGetPtr const & pvaClientGet)
{
     map<string,PvaClientGetPtr>::iterator iter = pvaClientGetMap.find(request);
     if(iter!=pvaClientGetMap.end()) {
         throw std::runtime_error("pvaClientGetCache::addGet pvaClientGet already cached");
     }
     pvaClientGetMap.insert(std::pair<string,PvaClientGetPtr>(request,pvaClientGet));
}

void PvaClientGetCache::showCache()
{
    map<string,PvaClientGetPtr>::iterator iter;
    for(iter = pvaClientGetMap.begin(); iter != pvaClientGetMap.end(); ++iter)
    {
         cout << "        " << iter->first << endl;
    }
}

size_t PvaClientGetCache::cacheSize()
{
    return pvaClientGetMap.size();

}

class PvaClientPutCache
{
public:
    PvaClientPutCache(){}
    ~PvaClientPutCache()
    {
         pvaClientPutMap.clear();
    }
    PvaClientPutPtr getPut(string const & request);
    void addPut(string const & request,PvaClientPutPtr const & pvaClientPut);
    void showCache();
    size_t cacheSize();
private:
    map<string,PvaClientPutPtr> pvaClientPutMap;
};


PvaClientPutPtr PvaClientPutCache::getPut(string const & request)
{
    map<string,PvaClientPutPtr>::iterator iter = pvaClientPutMap.find(request);
    if(iter!=pvaClientPutMap.end()) return iter->second;
    return PvaClientPutPtr();
}

void PvaClientPutCache::addPut(string const & request,PvaClientPutPtr const & pvaClientPut)
{
     map<string,PvaClientPutPtr>::iterator iter = pvaClientPutMap.find(request);
     if(iter!=pvaClientPutMap.end()) {
         throw std::runtime_error("pvaClientPutCache::addPut pvaClientPut already cached");
     }
     pvaClientPutMap.insert(std::pair<string,PvaClientPutPtr>(
         request,pvaClientPut));
}

void PvaClientPutCache::showCache()
{
    map<string,PvaClientPutPtr>::iterator iter;
    for(iter = pvaClientPutMap.begin(); iter != pvaClientPutMap.end(); ++iter)
    {
         cout << "        " << iter->first << endl;
    }
}

size_t PvaClientPutCache::cacheSize()
{
    return pvaClientPutMap.size();

}


PvaClientChannel::PvaClientChannel(
    PvaClientPtr const &pvaClient,
    string const & channelName,
    string const & providerName)
: pvaClient(pvaClient),
  channelName(channelName),
  providerName(providerName),
  connectState(connectIdle),
  isDestroyed(false),
  createRequest(CreateRequest::create()),
  pvaClientGetCache(new PvaClientGetCache()),
  pvaClientPutCache(new PvaClientPutCache())
{
    if(PvaClient::getDebug()) {
        cout << "PvaClientChannel::PvaClientChannel channelName " << channelName << endl;
    }
}

PvaClientChannel::~PvaClientChannel()
{
    if(PvaClient::getDebug()) {
        cout  << "PvaClientChannel::~PvaClientChannel() "
             << " channelName " << channelName
             << " this " << this << " channel " << channel
            << endl;
    }
    destroy();
}

void PvaClientChannel::destroy()
{
     {
        Lock xx(mutex);
        if(isDestroyed) return;
        isDestroyed = true;
    }
    if(PvaClient::getDebug()) {
        cout  << "PvaClientChannel::destroy() "
             << " channelName " << channelName
             << " this " << this << " channel " << channel
            << endl;
    }
    if(PvaClient::getDebug()) showCache();
    if(channel) channel->destroy();
    if(channel) channel.reset();
    pvaClientGetCache.reset();
    pvaClientPutCache.reset();
}


void PvaClientChannel::channelCreated(const Status& status, Channel::shared_pointer const & channel)
{
    if(PvaClient::getDebug()) {
        cout << "PvaClientChannel::channelCreated"
           << " channelName " << channelName
           << " isConnected " << (channel->isConnected() ? "true" : "false")
           << " status.isOK " << (status.isOK() ? "true" : "false")
           << " this " << this
           << endl;
    }
    Lock xx(mutex);
    if(isDestroyed) throw std::runtime_error("pvaClientChannel was destroyed");
    if(connectState!=connectActive) {
         string message("PvaClientChannel::channelCreated");
         message += " channel " + channelName
                 + " why was this called when connectState!=ConnectState.connectActive";
         throw std::runtime_error(message);
    }
    if(!status.isOK()) {
        string message("PvaClientChannel::channelCreated");
            + " status " +  status.getMessage() + " why??";
        throw std::runtime_error(message);
    }
    if(channel->isConnected()) {
        connectState = connected; 
        waitForConnect.signal();
    }
}

void PvaClientChannel::channelStateChange(
    Channel::shared_pointer const & channel,
    Channel::ConnectionState connectionState)
{
    if(PvaClient::getDebug()) {
        cout << " PvaClientChannel::channelStateChange "
        << " channelName " << channelName
        << " is connected " << (connectionState==Channel::CONNECTED ? "true" : "false")
        << " isDestroyed " << isDestroyed
        << " this " << this
        << endl;
    }
    Lock xx(mutex);
    if(isDestroyed) return;
    bool waitingForConnect = false;
    if(connectState==connectActive) waitingForConnect = true;
    if(connectionState!=Channel::CONNECTED) {
         string mess(channelName +
             " connection state " + Channel::ConnectionStateNames[connectionState]);
             message(mess,errorMessage);
             connectState = notConnected;
         return;
    } else {
        connectState = connected;
    }
    if(waitingForConnect) waitForConnect.signal();
}

string PvaClientChannel::getRequesterName()
{
    PvaClientPtr yyy = pvaClient.lock();
    if(!yyy) throw std::runtime_error(
         "PvaClientChannel::getRequesterName() PvaClient isDestroyed");
    return yyy->getRequesterName();
}

void PvaClientChannel::message(
    string const & message,
    MessageType messageType)
{
    PvaClientPtr yyy = pvaClient.lock();
    if(!yyy) throw std::runtime_error(
        "PvaClientChannel::message() pvaClient isDestroyed");
    yyy->message(channelName + " " + message, messageType);
}

string PvaClientChannel::getChannelName()
{
    if(isDestroyed) throw std::runtime_error(
        "PvaClientChannel::getChannelName() pvaClientChannel was destroyed");
    return channelName;
}

Channel::shared_pointer PvaClientChannel::getChannel()
{
    if(isDestroyed) throw std::runtime_error(
        "PvaClientChannel::getChannel() pvaClientChannel was destroyed");
    return channel;
}

void PvaClientChannel::connect(double timeout)
{
    if(isDestroyed) throw std::runtime_error(
        "PvaClientChannel::connect() pvaClientChannel was destroyed");
    if(PvaClient::getDebug()) {
        cout << "PvaClientChannel::connect"
        << " channelName " << channelName << endl;
    }
    issueConnect();
    Status status = waitConnect(timeout);
    if(status.isOK()) return;
    if(PvaClient::getDebug()) cout << "PvaClientChannel::connect  waitConnect failed\n";
    string message = string("channel ") + channelName 
                    + " PvaClientChannel::connect " + status.getMessage();
    throw std::runtime_error(message);
}

void PvaClientChannel::issueConnect()
{
    if(isDestroyed) throw std::runtime_error(
        "PvaClientChannel::issueConnect() pvaClientChannel was destroyed");
    if(PvaClient::getDebug()) {
        cout << "PvaClientChannel::issueConnect"
        << " channelName " << channelName << endl;
    }
    {
        Lock xx(mutex);
        if(isDestroyed) throw std::runtime_error("pvaClientChannel was destroyed");
        if(connectState!=connectIdle) {
           throw std::runtime_error("pvaClientChannel already connected");
        }
        connectState = connectActive;
    }
    ChannelProviderRegistry::shared_pointer reg = getChannelProviderRegistry();
    ChannelProvider::shared_pointer provider = reg->getProvider(providerName);
    if(!provider) {
        throw std::runtime_error(channelName + " provider " + providerName + " not registered");
    }
    ChannelRequester::shared_pointer channelRequester(shared_from_this());
    if(PvaClient::getDebug()) cout << "PvaClientChannel::issueConnect calling provider->createChannel\n";
    channel = provider->createChannel(channelName,channelRequester,ChannelProvider::PRIORITY_DEFAULT);
    if(!channel) {
         throw std::runtime_error(channelName + " channelCreate failed ");
    }
}

Status PvaClientChannel::waitConnect(double timeout)
{
    if(isDestroyed) throw std::runtime_error(
        "PvaClientChannel::waitConnect() pvaClientChannel was destroyed");
    if(PvaClient::getDebug()) {
        cout << "PvaClientChannel::waitConnect"
        << " channelName " << channelName << endl;
    }
    {
        Lock xx(mutex);
        if(isDestroyed) throw std::runtime_error("pvaClientChannel was destroyed");
        if(channel->isConnected()) return Status::Ok;
    }
    if(timeout>0.0) {
        waitForConnect.wait(timeout);
    } else {
        waitForConnect.wait();
    }
    if(channel->isConnected()) return Status::Ok;
    return Status(Status::STATUSTYPE_ERROR,channelName + " not connected");
}

PvaClientFieldPtr PvaClientChannel::createField()
{
    return createField("");
}

PvaClientFieldPtr PvaClientChannel::createField(string const & subField)
{
    if(connectState!=connected) connect(5.0);
    throw std::runtime_error("PvaClientChannel::createField not implemented");
}

PvaClientProcessPtr PvaClientChannel::createProcess()
{
    return createProcess("");
}

PvaClientProcessPtr PvaClientChannel::createProcess(string const & request)
{
    PVStructurePtr pvRequest = createRequest->createRequest(request);
    if(!pvRequest) {
        string message = string("channel ") + channelName 
            + " PvaClientChannel::createProcess invalid pvRequest: "
            + createRequest->getMessage();
        throw std::runtime_error(message);
    }
    return createProcess(pvRequest);
}

PvaClientProcessPtr PvaClientChannel::createProcess(PVStructurePtr const &  pvRequest)
{
    if(connectState!=connected) connect(5.0);
    PvaClientPtr yyy = pvaClient.lock();
    if(!yyy) throw std::runtime_error("PvaClient was destroyed");
    return PvaClientProcess::create(yyy,channel,pvRequest);
}

PvaClientGetPtr PvaClientChannel::get() {return get("value,alarm,timeStamp");}

PvaClientGetPtr PvaClientChannel::get(string const & request)
{
    PvaClientGetPtr pvaClientGet = pvaClientGetCache->getGet(request);
    if(pvaClientGet) return pvaClientGet;
    pvaClientGet = createGet(request);
    pvaClientGet->connect();
    pvaClientGetCache->addGet(request,pvaClientGet);
    return pvaClientGet;
}

PvaClientGetPtr PvaClientChannel::createGet()
{
    return PvaClientChannel::createGet("value,alarm,timeStamp");
}

PvaClientGetPtr PvaClientChannel::createGet(string const & request)
{
    PVStructurePtr pvRequest = createRequest->createRequest(request);
    if(!pvRequest) {
        string message = string("channel ") + channelName 
            + " PvaClientChannel::createGet invalid pvRequest: "
            + createRequest->getMessage();
        throw std::runtime_error(message);
    }
    return createGet(pvRequest);
}

PvaClientGetPtr PvaClientChannel::createGet(PVStructurePtr const &  pvRequest)
{
    if(connectState!=connected) connect(5.0);
    PvaClientPtr yyy = pvaClient.lock();
    if(!yyy) throw std::runtime_error("PvaClient was destroyed");
    return PvaClientGet::create(yyy,channel,pvRequest);
}

PvaClientPutPtr PvaClientChannel::put() {return put("value");}

PvaClientPutPtr PvaClientChannel::put(string const & request)
{
    PvaClientPutPtr pvaClientPut = pvaClientPutCache->getPut(request);
    if(pvaClientPut) return pvaClientPut;
    pvaClientPut = createPut(request);
    pvaClientPut->connect();
    pvaClientPut->get();
    pvaClientPutCache->addPut(request,pvaClientPut);
    return pvaClientPut;
}

PvaClientPutPtr PvaClientChannel::createPut()
{
    return createPut("value");
}

PvaClientPutPtr PvaClientChannel::createPut(string const & request)
{
    PVStructurePtr pvRequest = createRequest->createRequest(request);
    if(!pvRequest) {
        string message = string("channel ") + channelName 
            + " PvaClientChannel::createPut invalid pvRequest: "
            + createRequest->getMessage();
        throw std::runtime_error(message);
    }
    return createPut(pvRequest);
}

PvaClientPutPtr PvaClientChannel::createPut(PVStructurePtr const & pvRequest)
{
    if(connectState!=connected) connect(5.0);
    PvaClientPtr yyy = pvaClient.lock();
    if(!yyy) throw std::runtime_error("PvaClient was destroyed");
    return PvaClientPut::create(yyy,channel,pvRequest);
}

PvaClientPutGetPtr PvaClientChannel::createPutGet()
{
    return createPutGet("putField(argument)getField(result)");
}

PvaClientPutGetPtr PvaClientChannel::createPutGet(string const & request)
{
    PVStructurePtr pvRequest = createRequest->createRequest(request);
    if(!pvRequest) {
        string message = string("channel ") + channelName 
            + " PvaClientChannel::createPutGet invalid pvRequest: "
            + createRequest->getMessage();
        throw std::runtime_error(message);
    }
    return createPutGet(pvRequest);
}

PvaClientPutGetPtr PvaClientChannel::createPutGet(PVStructurePtr const & pvRequest)
{
    if(connectState!=connected) connect(5.0);
    PvaClientPtr yyy = pvaClient.lock();
    if(!yyy) throw std::runtime_error("PvaClient was destroyed");
    return PvaClientPutGet::create(yyy,channel,pvRequest);
}


PvaClientArrayPtr PvaClientChannel::createArray()
{
    return createArray("value");
}

PvaClientArrayPtr PvaClientChannel::createArray(string const & request)
{
    PVStructurePtr pvRequest = createRequest->createRequest(request);
    if(!pvRequest) {
        string message = string("channel ") + channelName 
            + " PvaClientChannel::createArray invalid pvRequest: "
            + createRequest->getMessage();
        throw std::runtime_error(message);
    }
    return createArray(pvRequest);
}

PvaClientArrayPtr PvaClientChannel::createArray(PVStructurePtr const &  pvRequest)
{
    if(connectState!=connected) connect(5.0);
    throw std::runtime_error("PvaClientChannel::createArray not implemented");
}


PvaClientMonitorPtr PvaClientChannel::monitor()
{
    return monitor("value,alarm,timeStamp");
}

PvaClientMonitorPtr PvaClientChannel::monitor(string const & request)
{
    PvaClientMonitorPtr pvaClientMonitor = createMonitor(request);
    pvaClientMonitor->connect();
    pvaClientMonitor->start();
    return pvaClientMonitor;
}

PvaClientMonitorPtr PvaClientChannel::monitor(PvaClientMonitorRequesterPtr const & pvaClientMonitorRequester)
{
      return monitor("value,alarm,timeStamp",pvaClientMonitorRequester);
}

PvaClientMonitorPtr PvaClientChannel::monitor(string const & request,
    PvaClientMonitorRequesterPtr const & pvaClientMonitorRequester)
{
    PvaClientMonitorPtr pvaClientMonitor = createMonitor(request);
    pvaClientMonitor->connect();
    pvaClientMonitor->setRequester(pvaClientMonitorRequester);
    pvaClientMonitor->start();
    return pvaClientMonitor;
}

PvaClientMonitorPtr PvaClientChannel::createMonitor()
{
    return createMonitor("value,alarm,timeStamp");
}

PvaClientMonitorPtr PvaClientChannel::createMonitor(string const & request)
{
    PVStructurePtr pvRequest = createRequest->createRequest(request);
    if(!pvRequest) {
        string message = string("channel ") + channelName 
            + " PvaClientChannel::createMonitor invalid pvRequest: "
            + createRequest->getMessage();
        throw std::runtime_error(message);
    }
    return createMonitor(pvRequest);
}

PvaClientMonitorPtr  PvaClientChannel::createMonitor(PVStructurePtr const &  pvRequest)
{
    if(connectState!=connected) connect(5.0);
    PvaClientPtr yyy = pvaClient.lock();
    if(!yyy) throw std::runtime_error("PvaClient was destroyed");
    return PvaClientMonitor::create(yyy,channel,pvRequest);
}

void PvaClientChannel::showCache()
{
     cout << "    pvaClientGet" << endl;
     pvaClientGetCache->showCache();
     cout << "    pvaClientPut" << endl;
     pvaClientPutCache->showCache();
}

size_t PvaClientChannel::cacheSize()
{
    return pvaClientGetCache->cacheSize() + pvaClientPutCache->cacheSize();
}


PvaClientChannelPtr PvaClientChannel::create(
   PvaClientPtr const &pvaClient,
   string const & channelName,
   string const & providerName)
{
    PvaClientChannelPtr channel(new PvaClientChannel(pvaClient,channelName,providerName));
    return channel;
}

}}

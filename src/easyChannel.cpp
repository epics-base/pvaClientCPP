/* easyChannel.cpp */
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


class EasyGetCache
{
public:
    EasyGetCache(){}
    ~EasyGetCache();
    void destroy() {
       easyGetMap.clear();
    }
    EasyGetPtr getGet(string const & request);
    void addGet(string const & request,EasyGetPtr const & easyGet);
private:
    map<string,EasyGetPtr> easyGetMap;
};

EasyGetCache::~EasyGetCache()
{
    destroy();
}

EasyGetPtr EasyGetCache::getGet(string const & request)
{
    map<string,EasyGetPtr>::iterator iter = easyGetMap.find(request);
    if(iter!=easyGetMap.end()) return iter->second;
    return EasyGetPtr();
}

void EasyGetCache::addGet(string const & request,EasyGetPtr const & easyGet)
{
     easyGetMap.insert(std::pair<string,EasyGetPtr>(
         request,easyGet));
}


class EasyPutCache
{
public:
    EasyPutCache(){}
    ~EasyPutCache();
    void destroy() {
       easyPutMap.clear();
    }
    EasyPutPtr getPut(string const & request);
    void addPut(string const & request,EasyPutPtr const & easyPut);
private:
    map<string,EasyPutPtr> easyPutMap;
};

EasyPutCache::~EasyPutCache()
{
    destroy();
}

EasyPutPtr EasyPutCache::getPut(string const & request)
{
    map<string,EasyPutPtr>::iterator iter = easyPutMap.find(request);
    if(iter!=easyPutMap.end()) return iter->second;
    return EasyPutPtr();
}

void EasyPutCache::addPut(string const & request,EasyPutPtr const & easyPut)
{
     easyPutMap.insert(std::pair<string,EasyPutPtr>(
         request,easyPut));
}

class ChannelRequesterImpl : public ChannelRequester
{
     EasyChannel *easyChannel;
public:
     ChannelRequesterImpl(EasyChannel *easyChannel)
     : easyChannel(easyChannel) {}
    void channelCreated(
        const Status& status,
        Channel::shared_pointer const & channel)
    { easyChannel->channelCreated(status,channel); }
    void channelStateChange(
        Channel::shared_pointer const & channel,
        Channel::ConnectionState connectionState)
    {easyChannel->channelStateChange(channel,connectionState);}
    tr1::shared_ptr<Channel> getChannel() {return easyChannel->getChannel();}
    string getRequesterName()
    {return easyChannel->getRequesterName();}
    void message(
        string const & message,
        MessageType messageType)
    { easyChannel->message(message,messageType); }
    void destroy() {easyChannel->destroy();}
};


EasyChannel::EasyChannel(
    EasyPVAPtr const &easyPVA,
    string const & channelName,
    string const & providerName)
: easyPVA(easyPVA),
  channelName(channelName),
  providerName(providerName),
  connectState(connectIdle),
  isDestroyed(false),
  createRequest(CreateRequest::create()),
  easyGetCache(new EasyGetCache()),
  easyPutCache(new EasyPutCache())
{}

EasyChannel::~EasyChannel()
{
    destroy();
}

void EasyChannel::channelCreated(const Status& status, Channel::shared_pointer const & channel)
{
    if(isDestroyed) throw std::runtime_error("easyChannel was destroyed");
    if(status.isOK()) {
        this->channel = channel;
        return;
    }
    cout << "EasyChannel::channelCreated status " << status.getMessage() << " why??\n";
}

void EasyChannel::channelStateChange(
    Channel::shared_pointer const & channel,
    Channel::ConnectionState connectionState)
{
    if(isDestroyed) return;
    bool waitingForConnect = false;
    if(connectState==connectActive) waitingForConnect = true;
    if(connectionState!=Channel::CONNECTED) {
         string mess(channelName +
             " connection state " + Channel::ConnectionStateNames[connectionState]);
         message(mess,errorMessage);
         channelConnectStatus = Status(Status::STATUSTYPE_ERROR,mess);
         connectState = notConnected;
    } else {
         connectState = connected;
    }
    if(waitingForConnect) waitForConnect.signal();
}

tr1::shared_ptr<Channel> EasyChannel::getChannel()
{
    if(isDestroyed) throw std::runtime_error("easyChannel was destroyed");
    return channel;
}

string EasyChannel::getRequesterName()
{
    EasyPVAPtr yyy = easyPVA.lock();
    if(!yyy) throw std::runtime_error("EasyPVA was destroyed");
    return yyy->getRequesterName();
}

void EasyChannel::message(
    string const & message,
    MessageType messageType)
{
    if(isDestroyed) throw std::runtime_error("easyChannel was destroyed");
    EasyPVAPtr yyy = easyPVA.lock();
    if(!yyy) throw std::runtime_error("EasyPVA was destroyed");
    yyy->message(message, messageType);
}

void EasyChannel::destroy()
{
    {
        Lock xx(mutex);
        if(isDestroyed) return;
        isDestroyed = true;
    }
    if(channel) channel->destroy();
    channel.reset();
    easyGetCache.reset();
    easyPutCache.reset();
}

string EasyChannel::getChannelName()
{
    if(isDestroyed) throw std::runtime_error("easyChannel was destroyed");
    return channelName;
}

void EasyChannel::connect(double timeout)
{
    if(isDestroyed) throw std::runtime_error("easyChannel was destroyed");
    issueConnect();
    Status status = waitConnect(timeout);
    if(status.isOK()) return;
    stringstream ss;
    ss << "channel " << getChannelName() << " EasyChannel::connect " << status.getMessage();
    throw std::runtime_error(ss.str());
}

void EasyChannel::issueConnect()
{
    if(isDestroyed) throw std::runtime_error("easyChannel was destroyed");
    if(connectState!=connectIdle) {
       throw std::runtime_error("easyChannel already connected");
    }
    channelRequester = ChannelRequester::shared_pointer(new ChannelRequesterImpl(this));

    channelConnectStatus = Status(Status::STATUSTYPE_ERROR,"createChannel failed");
    connectState = connectActive;
    ChannelProviderRegistry::shared_pointer reg = getChannelProviderRegistry();
    ChannelProvider::shared_pointer provider = reg->getProvider(providerName);
    channel = provider->createChannel(channelName,channelRequester,ChannelProvider::PRIORITY_DEFAULT);
    if(!channel) {
         throw std::runtime_error(channelConnectStatus.getMessage());
    }
}

Status EasyChannel::waitConnect(double timeout)
{
    if(isDestroyed) throw std::runtime_error("easyChannel was destroyed");
    waitForConnect.wait(timeout);
    if(connectState==connected) return Status::Ok;
    return Status(Status::STATUSTYPE_ERROR,channelConnectStatus.getMessage());
}

EasyFieldPtr EasyChannel::createField()
{
    return createField("");
}

EasyFieldPtr EasyChannel::createField(string const & subField)
{
    throw std::runtime_error("EasyChannel::createField not implemented");
}

EasyProcessPtr EasyChannel::createProcess()
{
    return createProcess("");
}

EasyProcessPtr EasyChannel::createProcess(string const & request)
{
    PVStructurePtr pvRequest = createRequest->createRequest(request);
    if(!pvRequest) {
        stringstream ss;
        ss << "channel " << getChannelName();
        ss << " EasyChannel::createProcess invalid pvRequest: " + createRequest->getMessage();
        throw std::runtime_error(ss.str());
    }
    return createProcess(pvRequest);
}

EasyProcessPtr EasyChannel::createProcess(PVStructurePtr const &  pvRequest)
{
    if(connectState!=connected) connect(5.0);
    if(connectState!=connected) throw std::runtime_error("EasyChannel::creatProcess not connected");
    EasyPVAPtr yyy = easyPVA.lock();
    if(!yyy) throw std::runtime_error("EasyPVA was destroyed");
    return EasyProcess::create(yyy,getPtrSelf(),channel,pvRequest);
}

EasyGetPtr EasyChannel::get() {return get("value,alarm,timeStamp");}

EasyGetPtr EasyChannel::get(string const & request)
{
    EasyGetPtr easyGet = easyGetCache->getGet(request);
    if(easyGet) return easyGet;
    easyGet = createGet(request);
    easyGet->connect();
    easyGetCache->addGet(request,easyGet);
    return easyGet;
}

EasyGetPtr EasyChannel::createGet()
{
    return EasyChannel::createGet("value,alarm,timeStamp");
}

EasyGetPtr EasyChannel::createGet(string const & request)
{
    PVStructurePtr pvRequest = createRequest->createRequest(request);
    if(!pvRequest) {
        stringstream ss;
        ss << "channel " << getChannelName();
        ss << " EasyChannel::createGet invalid pvRequest: " + createRequest->getMessage();
        throw std::runtime_error(ss.str());
    }
    return createGet(pvRequest);
}

EasyGetPtr EasyChannel::createGet(PVStructurePtr const &  pvRequest)
{
    if(connectState!=connected) connect(5.0);
    if(connectState!=connected) throw std::runtime_error("EasyChannel::creatGet not connected");
    EasyPVAPtr yyy = easyPVA.lock();
    if(!yyy) throw std::runtime_error("EasyPVA was destroyed");
    return EasyGet::create(yyy,getPtrSelf(),channel,pvRequest);
}

EasyPutPtr EasyChannel::put() {return put("value");}

EasyPutPtr EasyChannel::put(string const & request)
{
    EasyPutPtr easyPut = easyPutCache->getPut(request);
    if(easyPut) return easyPut;
    easyPut = createPut(request);
    easyPut->connect();
    easyPut->get();
    easyPutCache->addPut(request,easyPut);
    return easyPut;
}

EasyPutPtr EasyChannel::createPut()
{
    return createPut("value");
}

EasyPutPtr EasyChannel::createPut(string const & request)
{
    PVStructurePtr pvRequest = createRequest->createRequest(request);
    if(!pvRequest) {
        stringstream ss;
        ss << "channel " << getChannelName();
        ss << " EasyChannel::createPut invalid pvRequest: " + createRequest->getMessage();
        throw std::runtime_error(ss.str());
    }
    return createPut(pvRequest);
}

EasyPutPtr EasyChannel::createPut(PVStructurePtr const & pvRequest)
{
    if(connectState!=connected) connect(5.0);
    if(connectState!=connected) throw std::runtime_error("EasyChannel::creatPut not connected");
    EasyPVAPtr yyy = easyPVA.lock();
    if(!yyy) throw std::runtime_error("EasyPVA was destroyed");
    return EasyPut::create(yyy,getPtrSelf(),channel,pvRequest);
}

EasyPutGetPtr EasyChannel::createPutGet()
{
    return createPutGet("putField(argument)getField(result)");
}

EasyPutGetPtr EasyChannel::createPutGet(string const & request)
{
    PVStructurePtr pvRequest = createRequest->createRequest(request);
    if(!pvRequest) {
        stringstream ss;
        ss << "channel " << getChannelName();
        ss << " EasyChannel::createPutGet invalid pvRequest: " + createRequest->getMessage();
        throw std::runtime_error(ss.str());
    }
    return createPutGet(pvRequest);
}

EasyPutGetPtr EasyChannel::createPutGet(PVStructurePtr const & pvRequest)
{
    if(connectState!=connected) connect(5.0);
    if(connectState!=connected) throw std::runtime_error("EasyChannel::creatPutGet not connected");
    EasyPVAPtr yyy = easyPVA.lock();
    if(!yyy) throw std::runtime_error("EasyPVA was destroyed");
    return EasyPutGet::create(yyy,getPtrSelf(),channel,pvRequest);
}

EasyRPCPtr EasyChannel::createRPC()
{
    return createRPC("");
}

EasyRPCPtr EasyChannel::createRPC(string const & request)
{
    PVStructurePtr pvRequest = createRequest->createRequest(request);
    if(!pvRequest) {
        stringstream ss;
        ss << "channel " << getChannelName();
        ss << " EasyChannel::createRPC invalid pvRequest: " + createRequest->getMessage();
        throw std::runtime_error(ss.str());
    }
    return createRPC(pvRequest);
}

EasyRPCPtr EasyChannel::createRPC(PVStructurePtr const & pvRequest)
{
    throw std::runtime_error("EasyChannel::createRPC not implemented");
}

EasyArrayPtr EasyChannel::createArray()
{
    return createArray("value");
}

EasyArrayPtr EasyChannel::createArray(string const & request)
{
    PVStructurePtr pvRequest = createRequest->createRequest(request);
    if(!pvRequest) {
        stringstream ss;
        ss << "channel " << getChannelName();
        ss << " EasyChannel::createArray invalid pvRequest: " + createRequest->getMessage();
        throw std::runtime_error(ss.str());
    }
    return createArray(pvRequest);
}

EasyArrayPtr EasyChannel::createArray(PVStructurePtr const &  pvRequest)
{
    throw std::runtime_error("EasyChannel::createArray not implemented");
}


EasyMonitorPtr EasyChannel::monitor() {return monitor("value,alarm,timeStamp");}

EasyMonitorPtr EasyChannel::monitor(string const & request)
{
    EasyMonitorPtr easyMonitor = createMonitor(request);
    easyMonitor->connect();
    easyMonitor->start();
    return easyMonitor;
}

EasyMonitorPtr EasyChannel::monitor(EasyMonitorRequesterPtr const & easyMonitorRequester)
{    return monitor("value,alarm,timeStamp",easyMonitorRequester);
}

EasyMonitorPtr EasyChannel::monitor(string const & request,
    EasyMonitorRequesterPtr const & easyMonitorRequester)
{
    EasyMonitorPtr easyMonitor = createMonitor(request);
    easyMonitor->connect();
    easyMonitor->setRequester(easyMonitorRequester);
    easyMonitor->start();
    return easyMonitor;
}

EasyMonitorPtr EasyChannel::createMonitor()
{
    return createMonitor("value,alarm,timeStamp");
}

EasyMonitorPtr EasyChannel::createMonitor(string const & request)
{
    PVStructurePtr pvRequest = createRequest->createRequest(request);
    if(!pvRequest) {
        stringstream ss;
        ss << "channel " << getChannelName();
        ss << " EasyChannel::createMonitor invalid pvRequest: " + createRequest->getMessage();
        throw std::runtime_error(ss.str());
    }
    return createMonitor(pvRequest);
}

EasyMonitorPtr  EasyChannel::createMonitor(PVStructurePtr const &  pvRequest)
{
    if(connectState!=connected) connect(5.0);
    if(connectState!=connected) throw std::runtime_error("EasyChannel::createMonitor not connected");
    EasyPVAPtr yyy = easyPVA.lock();
    if(!yyy) throw std::runtime_error("EasyPVA was destroyed");
    return EasyMonitor::create(yyy,getPtrSelf(),channel,pvRequest);
}


EasyChannelPtr EasyChannel::create(
   EasyPVAPtr const &easyPVA,
   string const & channelName,
   string const & providerName)
{
    EasyChannelPtr channel(new EasyChannel(easyPVA,channelName,providerName));
    return channel;
}

}}

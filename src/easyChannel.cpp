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

class epicsShareClass EasyChannelImpl :
    public EasyChannel,
    public std::tr1::enable_shared_from_this<EasyChannelImpl>
{
public:
    EasyChannelImpl(
        EasyPVAPtr const &pva,
        string const & channelName,
        string const & providerName);
    ~EasyChannelImpl();
    // from EasyChannel
    void channelCreated(const Status& status, Channel::shared_pointer const & channel);
    void channelStateChange(
        Channel::shared_pointer const & channel,
        Channel::ConnectionState connectionState);
    tr1::shared_ptr<Channel> getChannel();
    string getRequesterName();
    void message(
        string const & message,
        MessageType messageType);
    virtual void destroy();
    virtual string getChannelName();
    virtual void connect(double timeout);
    virtual void issueConnect();
    virtual Status waitConnect(double timeout);
    virtual EasyFieldPtr createField();
    virtual EasyFieldPtr createField(string const & subField);
    virtual EasyProcessPtr createProcess();
    virtual EasyProcessPtr createProcess(string const & request);
    virtual EasyProcessPtr createProcess(PVStructurePtr const &  pvRequest);
    virtual EasyGetPtr createGet();
    virtual EasyGetPtr createGet(string const & request);
    virtual EasyGetPtr createGet(PVStructurePtr const &  pvRequest);
    virtual EasyPutPtr createPut();
    virtual EasyPutPtr createPut(string const & request);
    virtual EasyPutPtr createPut(PVStructurePtr const & pvRequest);
    virtual EasyPutGetPtr createPutGet();
    virtual EasyPutGetPtr createPutGet(string const & request);
    virtual EasyPutGetPtr createPutGet(PVStructurePtr const & pvRequest);
    virtual EasyRPCPtr createRPC();
    virtual EasyRPCPtr createRPC(string const & request);
    virtual EasyRPCPtr createRPC(PVStructurePtr const & pvRequest);
    virtual EasyArrayPtr createArray();
    virtual EasyArrayPtr createArray(string const & request);
    virtual EasyArrayPtr createArray(PVStructurePtr const &  pvRequest);
    virtual EasyMonitorPtr createMonitor();
    virtual EasyMonitorPtr createMonitor(string const & request);
    virtual EasyMonitorPtr createMonitor(PVStructurePtr const &  pvRequest);

    EasyChannelPtr getPtrSelf()
    {
        return shared_from_this();
    }
private:
    enum ConnectState {connectIdle,connectActive,notConnected,connected};

    EasyPVAPtr easyPVA;
    string channelName;
    string providerName;
    ConnectState connectState;
    bool isDestroyed;
    CreateRequest::shared_pointer createRequest;

    Status channelConnectStatus;
    Mutex mutex;
    Event waitForConnect;
    Channel::shared_pointer channel;
    ChannelRequester::shared_pointer channelRequester;

};

namespace easyChannel {
class ChannelRequesterImpl : public ChannelRequester
{
     EasyChannelImpl *easyChannel;
public:
     ChannelRequesterImpl(EasyChannelImpl *easyChannel)
     : easyChannel(easyChannel) {}
    virtual void channelCreated(
        const Status& status,
        Channel::shared_pointer const & channel)
    { easyChannel->channelCreated(status,channel); }
    virtual void channelStateChange(
        Channel::shared_pointer const & channel,
        Channel::ConnectionState connectionState)
    {easyChannel->channelStateChange(channel,connectionState);}
    virtual tr1::shared_ptr<Channel> getChannel() {return easyChannel->getChannel();}
    virtual string getRequesterName()
    {return easyChannel->getRequesterName();}
    virtual void message(
        string const & message,
        MessageType messageType)
    { easyChannel->message(message,messageType); }
    virtual void destroy() {easyChannel->destroy();}
};
} //end namespace easyChannel`

using  namespace epics::easyPVA::easyChannel;

EasyChannelImpl::EasyChannelImpl(
    EasyPVAPtr const &easyPVA,
    string const & channelName,
    string const & providerName)
: easyPVA(easyPVA),
  channelName(channelName),
  providerName(providerName),
  connectState(connectIdle),
  isDestroyed(false),
  createRequest(CreateRequest::create())
{}

EasyChannelImpl::~EasyChannelImpl()
{
    destroy();
}

void EasyChannelImpl::channelCreated(const Status& status, Channel::shared_pointer const & channel)
{
    if(isDestroyed) throw std::runtime_error("easyChannel was destroyed");
    channelConnectStatus = status;
    this->channel = channel;
}

void EasyChannelImpl::channelStateChange(
    Channel::shared_pointer const & channel,
    Channel::ConnectionState connectionState)
{
    if(isDestroyed) throw std::runtime_error("easyChannel was destroyed");
    bool waitingForConnect = false;
    if(connectState==connectActive) waitingForConnect = true;
    if(connectionState!=Channel::CONNECTED) {
         string mess(channelName + " connection state " + Channel::ConnectionStateNames[connectionState]);
         message(mess,errorMessage);
         channelConnectStatus = Status(Status::STATUSTYPE_ERROR,mess);
         connectState = notConnected;
    } else {
         connectState = connected;
    }
    if(waitingForConnect) waitForConnect.signal();
}

tr1::shared_ptr<Channel> EasyChannelImpl::getChannel()
{
    if(isDestroyed) throw std::runtime_error("easyChannel was destroyed");
    return channel;
}

string EasyChannelImpl::getRequesterName()
{
    return easyPVA->getRequesterName();
}

void EasyChannelImpl::message(
    string const & message,
    MessageType messageType)
{
    if(isDestroyed) throw std::runtime_error("easyChannel was destroyed");
    easyPVA->message(message, messageType);
}

void EasyChannelImpl::destroy()
{
    {
        Lock xx(mutex);
        if(isDestroyed) return;
        isDestroyed = true;
    }
    if(channel) channel->destroy();
    channel.reset();
}

string EasyChannelImpl::getChannelName()
{
    if(isDestroyed) throw std::runtime_error("easyChannel was destroyed");
    return channelName;
}

void EasyChannelImpl::connect(double timeout)
{
    if(isDestroyed) throw std::runtime_error("easyChannel was destroyed");
    issueConnect();
    Status status = waitConnect(timeout);
    if(status.isOK()) return;
    stringstream ss;
    ss << "channel " << getChannelName() << " EasyChannel::connect " << status.getMessage();
    throw std::runtime_error(ss.str());
}

void EasyChannelImpl::issueConnect()
{
    if(isDestroyed) throw std::runtime_error("easyChannel was destroyed");
    if(connectState!=connectIdle) {
       throw std::runtime_error("easyChannel already connected");
    }
    channelRequester = ChannelRequester::shared_pointer(new ChannelRequesterImpl(this));

    connectState = connectActive;
    ChannelProviderRegistry::shared_pointer reg = getChannelProviderRegistry();
    ChannelProvider::shared_pointer provider = reg->getProvider(providerName);
    channel = provider->createChannel(channelName,channelRequester,ChannelProvider::PRIORITY_DEFAULT);
}

Status EasyChannelImpl::waitConnect(double timeout)
{
    if(isDestroyed) throw std::runtime_error("easyChannel was destroyed");
    waitForConnect.wait(timeout);
    if(connectState==connected) return Status::Ok;
    return Status(Status::STATUSTYPE_ERROR,channelConnectStatus.getMessage());
}

EasyFieldPtr EasyChannelImpl::createField()
{
    return createField("");
}

EasyFieldPtr EasyChannelImpl::createField(string const & subField)
{
    throw std::runtime_error("EasyChannel::createField not implemented");
}

EasyProcessPtr EasyChannelImpl::createProcess()
{
    return createProcess("");
}

EasyProcessPtr EasyChannelImpl::createProcess(string const & request)
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

EasyProcessPtr EasyChannelImpl::createProcess(PVStructurePtr const &  pvRequest)
{
    throw std::runtime_error("EasyChannel::createProcess not implemented");
}

EasyGetPtr EasyChannelImpl::createGet()
{
    return EasyChannelImpl::createGet("value,alarm.timeStamp");
}

EasyGetPtr EasyChannelImpl::createGet(string const & request)
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

EasyGetPtr EasyChannelImpl::createGet(PVStructurePtr const &  pvRequest)
{
    if(connectState!=connected) connect(5.0);
    if(connectState!=connected) throw std::runtime_error("EasyChannel::creatGet not connected");
    return EasyGetFactory::createEasyGet(easyPVA,getPtrSelf(),channel,pvRequest);
}

EasyPutPtr EasyChannelImpl::createPut()
{
    return createPut("value");
}

EasyPutPtr EasyChannelImpl::createPut(string const & request)
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

EasyPutPtr EasyChannelImpl::createPut(PVStructurePtr const & pvRequest)
{
    throw std::runtime_error("EasyChannel::createPut not implemented");
}

EasyPutGetPtr EasyChannelImpl::createPutGet()
{
    return createPutGet("putField(argument)getField(result)");
}

EasyPutGetPtr EasyChannelImpl::createPutGet(string const & request)
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

EasyPutGetPtr EasyChannelImpl::createPutGet(PVStructurePtr const & pvRequest)
{
    throw std::runtime_error("EasyChannel::createPutGet not implemented");
}

EasyRPCPtr EasyChannelImpl::createRPC()
{
    return createRPC("");
}

EasyRPCPtr EasyChannelImpl::createRPC(string const & request)
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

EasyRPCPtr EasyChannelImpl::createRPC(PVStructurePtr const & pvRequest)
{
    throw std::runtime_error("EasyChannel::createRPC not implemented");
}

EasyArrayPtr EasyChannelImpl::createArray()
{
    return createArray("value");
}

EasyArrayPtr EasyChannelImpl::createArray(string const & request)
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

EasyArrayPtr EasyChannelImpl::createArray(PVStructurePtr const &  pvRequest)
{
    throw std::runtime_error("EasyChannel::createArray not implemented");
}

EasyMonitorPtr EasyChannelImpl::createMonitor()
{
    return createMonitor("value,alarm,timeStamp");
}

EasyMonitorPtr EasyChannelImpl::createMonitor(string const & request)
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

EasyMonitorPtr  EasyChannelImpl::createMonitor(PVStructurePtr const &  pvRequest)
{
    throw std::runtime_error("EasyChannel::createMonitor not implemented");
}


EasyChannelPtr EasyChannelFactory::createEasyChannel(
   EasyPVAPtr const &easyPVA,
   string const & channelName)
{
    return EasyChannelFactory::createEasyChannel(easyPVA,channelName,"pva");
}

EasyChannelPtr EasyChannelFactory::createEasyChannel(
   EasyPVAPtr const &easyPVA,
   string const & channelName,
   string const & providerName)
{
    EasyChannelPtr channel(new EasyChannelImpl(easyPVA,channelName,providerName));
    return channel;
}

}}

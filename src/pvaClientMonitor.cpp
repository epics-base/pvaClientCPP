/* pvaClientMonitor.cpp */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvData is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
/**
 * @author mrk
 * @date 2015.03
 */

#include <sstream>
#include <pv/event.h>
#include <pv/bitSetUtil.h>

#define epicsExportSharedSymbols

#include <pv/pvaClient.h>

using std::tr1::static_pointer_cast;
using std::tr1::dynamic_pointer_cast;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace std;

namespace epics { namespace pvaClient {

class MonitorRequesterImpl : public MonitorRequester
{
    PvaClientMonitor::weak_pointer pvaClientMonitor;
    PvaClient::weak_pointer pvaClient;
public:
    MonitorRequesterImpl(
        PvaClientMonitorPtr const & pvaClientMonitor,
        PvaClientPtr const &pvaClient)
    : pvaClientMonitor(pvaClientMonitor),
      pvaClient(pvaClient)
    {}
    virtual ~MonitorRequesterImpl() {
        if(PvaClient::getDebug()) std::cout << "~MonitorRequesterImpl" << std::endl;
    }

    virtual std::string getRequesterName() {
        PvaClientMonitorPtr clientMonitor(pvaClientMonitor.lock());
        if(!clientMonitor) return string("pvaClientMonitor is null");
        return clientMonitor->getRequesterName();
    }

    virtual void message(std::string const & message, epics::pvData::MessageType messageType) {
        PvaClientMonitorPtr clientMonitor(pvaClientMonitor.lock());
        if(!clientMonitor) return;
        clientMonitor->message(message,messageType);
    }

    virtual void monitorConnect(
        const Status& status,
        Monitor::shared_pointer const & monitor,
        Structure::const_shared_pointer const & structure)
    {
        PvaClientMonitorPtr clientMonitor(pvaClientMonitor.lock());
        if(!clientMonitor) return;
        clientMonitor->monitorConnect(status,monitor,structure);  
    }

    virtual void unlisten(epics::pvData::MonitorPtr const & monitor)
    {
        PvaClientMonitorPtr clientMonitor(pvaClientMonitor.lock());
        if(!clientMonitor) return;
        clientMonitor->unlisten(monitor);  
    }

    virtual void monitorEvent(epics::pvData::MonitorPtr const & monitor)
    {
        PvaClientMonitorPtr clientMonitor(pvaClientMonitor.lock());
        if(!clientMonitor) return;
        clientMonitor->monitorEvent(monitor);  
    }
};


PvaClientMonitorPtr PvaClientMonitor::create(
        PvaClientPtr const &pvaClient,
        Channel::shared_pointer const & channel,
        PVStructurePtr const &pvRequest)
{
    PvaClientMonitorPtr epv(new PvaClientMonitor(pvaClient,channel,pvRequest));
    epv->monitorRequester = MonitorRequesterImplPtr(
        new MonitorRequesterImpl(epv,pvaClient));
    return epv;
}

PvaClientMonitor::PvaClientMonitor(
        PvaClientPtr const &pvaClient,
        Channel::shared_pointer const & channel,
        PVStructurePtr const &pvRequest)
: pvaClient(pvaClient),
  channel(channel),
  pvRequest(pvRequest),
  isDestroyed(false),
  connectState(connectIdle),
  userPoll(false),
  userWait(false)
{
    if(PvaClient::getDebug()) {
         cout<< "PvaClientMonitor::PvaClientMonitor()"
             << " channelName " << channel->getChannelName() 
             << endl;
    }
}

PvaClientMonitor::~PvaClientMonitor()
{
    if(PvaClient::getDebug()) cout<< "PvaClientMonitor::~PvaClientMonitor\n";
    {
        Lock xx(mutex);
        if(isDestroyed) throw std::runtime_error("pvaClientMonitor was destroyed");
        isDestroyed = true;
    }
    if(PvaClient::getDebug()) {
        string channelName("disconnected");
        Channel::shared_pointer chan(channel.lock());
        if(chan) channelName = chan->getChannelName();
        cout<< "PvaClientMonitor::~PvaClientMonitor"
           << " channelName " << channelName
           << endl;
    }
    if(monitor) {
       if(connectState==monitorStarted) monitor->stop();
       monitor->destroy();
    }
}

void PvaClientMonitor::checkMonitorState()
{
    if(connectState==connectIdle) connect();
    if(connectState==connected) start();
}

string PvaClientMonitor::getRequesterName()
{
     PvaClientPtr yyy = pvaClient.lock();
     if(!yyy) return string("PvaClientMonitor::getRequesterName() PvaClient isDestroyed");
     return yyy->getRequesterName();
}

void PvaClientMonitor::message(string const & message,MessageType messageType)
{
    PvaClientPtr yyy = pvaClient.lock();
    if(!yyy) return;
    yyy->message(message, messageType);
}

void PvaClientMonitor::monitorConnect(
    const Status& status,
    Monitor::shared_pointer const & monitor,
    StructureConstPtr const & structure)
{
    Channel::shared_pointer chan(channel.lock());
    if(PvaClient::getDebug()) {
        string channelName("disconnected");
        Channel::shared_pointer chan(channel.lock());
        if(chan) channelName = chan->getChannelName();
        cout << "PvaClientMonitor::monitorConnect"
           << " channelName " << channelName
           << " status.isOK " << (status.isOK() ? "true" : "false")
           << endl;
    }
    connectStatus = status;
    connectState = connected;
    this->monitor = monitor;
    if(status.isOK() && chan) {
        pvaClientData = PvaClientMonitorData::create(structure);
        pvaClientData->setMessagePrefix(chan->getChannelName());
    }
    waitForConnect.signal();
    
}

void PvaClientMonitor::monitorEvent(MonitorPtr const & monitor)
{
    if(PvaClient::getDebug()) {
        string channelName("disconnected");
        Channel::shared_pointer chan(channel.lock());
        if(chan) channelName = chan->getChannelName();
        cout << "PvaClientMonitor::monitorEvent"
           << " channelName " << channelName
           << endl;
    }    
    PvaClientMonitorRequesterPtr req = pvaClientMonitorRequester.lock();
    if(req) req->event(shared_from_this());
    if(userWait) waitForEvent.signal();
}

void PvaClientMonitor::unlisten(MonitorPtr const & monitor)
{
    if(PvaClient::getDebug()) cout << "PvaClientMonitor::unlisten\n";
    throw std::runtime_error("pvaClientMonitor::unlisten called but do not know what to do");
}

void PvaClientMonitor::connect()
{
    issueConnect();
    Status status = waitConnect();
    if(status.isOK()) return;
    Channel::shared_pointer chan(channel.lock());
    string channelName("disconnected");
    if(chan) channelName = chan->getChannelName();
    string message = string("channel ") 
        + channelName
        + " PvaClientMonitor::connect "
        + status.getMessage();
    throw std::runtime_error(message);
}

void PvaClientMonitor::issueConnect()
{
    Channel::shared_pointer chan(channel.lock());
    if(connectState!=connectIdle) {
        string channelName("disconnected");
        if(chan) channelName = chan->getChannelName();
        string message = string("channel ")
            + channelName 
            + " pvaClientMonitor already connected ";
        throw std::runtime_error(message);
    }
    if(chan) {
        connectState = connectActive;
        monitor = chan->createMonitor(monitorRequester,pvRequest);
        return;
    }
    throw std::runtime_error("PvaClientMonitor::issueConnect() but channel disconnected");
}

Status PvaClientMonitor::waitConnect()
{
    if(connectState==connected) {
         if(connectStatus.isOK()) connectState = connectIdle;
         return connectStatus;
    }
    if(connectState!=connectActive) {
        Channel::shared_pointer chan(channel.lock());
        string channelName("disconnected");
        if(chan) channelName = chan->getChannelName();
        string message = string("channel ")
            + channelName
            + " PvaClientMonitor::waitConnect illegal connect state ";
        throw std::runtime_error(message);
    }
    waitForConnect.wait();
    connectState = connectStatus.isOK() ? connected : connectIdle;
    return connectStatus;
}

void PvaClientMonitor::setRequester(PvaClientMonitorRequesterPtr const & pvaClientMonitorrRequester)
{
    if(PvaClient::getDebug()) {
        string channelName("disconnected");
        Channel::shared_pointer chan(channel.lock());
        if(chan) channelName = chan->getChannelName();
        cout << "PvaClientMonitor::setRequester"
           << " channelName " << channelName
           << endl;
    }
    this->pvaClientMonitorRequester = pvaClientMonitorrRequester;
}

void PvaClientMonitor::start()
{
    if(PvaClient::getDebug()) {
        string channelName("disconnected");
        Channel::shared_pointer chan(channel.lock());
        if(chan) channelName = chan->getChannelName();
        cout << "PvaClientMonitor::start"
           << " channelName " << channelName
           << endl;
    }
    if(connectState==monitorStarted) return;
    if(connectState==connectIdle) connect();
    if(connectState!=connected) {
        Channel::shared_pointer chan(channel.lock());
        string channelName("disconnected");
        if(chan) channelName = chan->getChannelName();
        string message = string("channel ") + channelName
            + " PvaClientMonitor::start illegal state ";
        throw std::runtime_error(message);
    }
    connectState = monitorStarted;
    monitor->start();
}


void PvaClientMonitor::stop()
{
    if(PvaClient::getDebug()) {
        string channelName("disconnected");
        Channel::shared_pointer chan(channel.lock());
        if(chan) channelName = chan->getChannelName();
        cout << "PvaClientMonitor::stop"
           << " channelName " << channelName
           << endl;
    }
    if(connectState!=monitorStarted) return;
    connectState = connected;
    monitor->stop();
}

bool PvaClientMonitor::poll()
{
    if(PvaClient::getDebug()) {
        string channelName("disconnected");
        Channel::shared_pointer chan(channel.lock());
        if(chan) channelName = chan->getChannelName();
        cout << "PvaClientMonitor::poll"
           << " channelName " << channelName
           << endl;
    }
    checkMonitorState();
    if(connectState!=monitorStarted) {
        string channelName("disconnected");
        Channel::shared_pointer chan(channel.lock());
        if(chan) channelName = chan->getChannelName();
        string message = string("channel ") + channelName
            + " PvaClientMonitor::poll illegal state ";
         throw std::runtime_error(message);
    }
    if(userPoll) {
                string channelName("disconnected");
        Channel::shared_pointer chan(channel.lock());
        if(chan) channelName = chan->getChannelName();
        string message = string("channel ") + channelName
            + " PvaClientMonitor::poll did not release last";
        throw std::runtime_error(message);
    }
    monitorElement = monitor->poll();
    if(!monitorElement) return false;
    userPoll = true;
    pvaClientData->setData(monitorElement);
   return true;
}

bool PvaClientMonitor::waitEvent(double secondsToWait)
{
    if(PvaClient::getDebug()) {
        string channelName("disconnected");
        Channel::shared_pointer chan(channel.lock());
        if(chan) channelName = chan->getChannelName();
        cout << "PvaClientMonitor::waitEvent"
           << " channelName " << channelName
           << endl;
    }
    if(connectState!=monitorStarted) {
        Channel::shared_pointer chan(channel.lock());
        string channelName("disconnected");
        if(chan) channelName = chan->getChannelName();
        string message = string("channel ") + channelName
            + " PvaClientMonitor::waitEvent illegal state ";
        throw std::runtime_error(message);
    }
    if(poll()) return true;
    userWait = true;
    if(secondsToWait==0.0) {
        waitForEvent.wait();
    } else {
        waitForEvent.wait(secondsToWait);
    }
    userWait = false;
    return poll();
}

void PvaClientMonitor::releaseEvent()
{
    if(PvaClient::getDebug()) {
        string channelName("disconnected");
        Channel::shared_pointer chan(channel.lock());
        if(chan) channelName = chan->getChannelName();
        cout << "PvaClientMonitor::releaseEvent"
           << " channelName " << channelName
           << endl;
    }
    if(connectState!=monitorStarted) {
        Channel::shared_pointer chan(channel.lock());
        string channelName("disconnected");
        if(chan) channelName = chan->getChannelName();
        string message = string("channel ") + channelName
            + " PvaClientMonitor::releaseEvent monitor not started ";
        throw std::runtime_error(message);
    }
    if(!userPoll) {
        Channel::shared_pointer chan(channel.lock());
        string channelName("disconnected");
        if(chan) channelName = chan->getChannelName();
        string message = string("channel ") + channelName
            + " PvaClientMonitor::releaseEvent did not call poll";
        throw std::runtime_error(message);
    }
    userPoll = false;
    monitor->release(monitorElement);
}

PvaClientMonitorDataPtr PvaClientMonitor::getData()
{
    if(PvaClient::getDebug()) {
        string channelName("disconnected");
        Channel::shared_pointer chan(channel.lock());
        if(chan) channelName = chan->getChannelName();
        cout << "PvaClientMonitor::getData"
           << " channelName " << channelName
           << endl;
    }
    checkMonitorState();
    return pvaClientData;
}


}}

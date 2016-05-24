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
    if(PvaClient::getDebug()) cout<< "PvaClientMonitor::PvaClientMonitor()\n";
}

PvaClientMonitor::~PvaClientMonitor()
{
    if(PvaClient::getDebug()) cout<< "PvaClientMonitor::~PvaClientMonitor()\n";
    {
        Lock xx(mutex);
        if(isDestroyed) throw std::runtime_error("pvaClientMonitor was destroyed");
        isDestroyed = true;
    }
    if(monitor) monitor->destroy();
}

void PvaClientMonitor::checkMonitorState()
{
    if(PvaClient::getDebug()) cout<< "PvaClientMonitor::checkMonitorState()\n";
    if(connectState==connectIdle) connect();
    if(connectState==connected) start();
}

string PvaClientMonitor::getRequesterName()
{
     if(PvaClient::getDebug()) cout<< "PvaClientMonitor::getRequesterName()\n";
     PvaClientPtr yyy = pvaClient.lock();
     if(!yyy) throw std::runtime_error("pvaClient was destroyed");
     return yyy->getRequesterName();
}

void PvaClientMonitor::message(string const & message,MessageType messageType)
{
    if(PvaClient::getDebug()) cout<< "PvaClientMonitor::message()\n";
    PvaClientPtr yyy = pvaClient.lock();
    if(!yyy) throw std::runtime_error("pvaClient was destroyed");
    yyy->message(message, messageType);
}

void PvaClientMonitor::monitorConnect(
    const Status& status,
    Monitor::shared_pointer const & monitor,
    StructureConstPtr const & structure)
{
    if(PvaClient::getDebug()) cout<< "PvaClientMonitor::monitorConnect()\n";
    connectStatus = status;
    connectState = connected;
    this->monitor = monitor;
    if(status.isOK()) {
        pvaClientData = PvaClientMonitorData::create(structure);
        pvaClientData->setMessagePrefix(channel->getChannelName());
    }
    waitForConnect.signal();
    
}

void PvaClientMonitor::monitorEvent(MonitorPtr const & monitor)
{
    if(PvaClient::getDebug()) cout<< "PvaClientMonitor::monitorEvent()\n";
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
    if(PvaClient::getDebug()) cout << "PvaClientMonitor::connect\n";
    issueConnect();
    Status status = waitConnect();
    if(status.isOK()) return;
    string message = string("channel ") + channel->getChannelName() 
         + " PvaClientMonitor::connect " + status.getMessage();
    throw std::runtime_error(message);
}

void PvaClientMonitor::issueConnect()
{
    if(PvaClient::getDebug()) cout << "PvaClientMonitor::issueConnect\n";
    if(connectState!=connectIdle) {
        string message = string("channel ") + channel->getChannelName() 
            + " pvaClientMonitor already connected ";
        throw std::runtime_error(message);
    }
    connectState = connectActive;
    monitor = channel->createMonitor(monitorRequester,pvRequest);
}

Status PvaClientMonitor::waitConnect()
{
    if(PvaClient::getDebug()) cout << "PvaClientMonitor::waitConnect\n";
    if(connectState==connected) {
         if(connectStatus.isOK()) connectState = connectIdle;
         return connectStatus;
    }
    if(connectState!=connectActive) {
        string message = string("channel ") + channel->getChannelName() 
            + " pvaClientMonitor illegal connect state ";
        throw std::runtime_error(message);
    }
    waitForConnect.wait();
    connectState = connectStatus.isOK() ? connected : connectIdle;
    return connectStatus;
}

void PvaClientMonitor::setRequester(PvaClientMonitorRequesterPtr const & pvaClientMonitorrRequester)
{
    if(PvaClient::getDebug()) cout << "PvaClientMonitor::setRequester\n";
    this->pvaClientMonitorRequester = pvaClientMonitorrRequester;
}

void PvaClientMonitor::start()
{
    if(PvaClient::getDebug()) cout << "PvaClientMonitor::start\n";
    if(connectState==monitorStarted) return;
    if(connectState==connectIdle) connect();
    if(connectState!=connected) throw std::runtime_error("PvaClientMonitor::start illegal state");
    connectState = monitorStarted;
    monitor->start();
}


void PvaClientMonitor::stop()
{
    if(PvaClient::getDebug()) cout << "PvaClientMonitor::stop\n";
    if(connectState!=monitorStarted) return;
    connectState = connected;
    monitor->stop();
}

bool PvaClientMonitor::poll()
{
    if(PvaClient::getDebug()) cout << "PvaClientMonitor::poll\n";
    checkMonitorState();
    if(connectState!=monitorStarted) throw std::runtime_error("PvaClientMonitor::poll illegal state");
    if(userPoll) throw std::runtime_error("PvaClientMonitor::poll did not release last");
    monitorElement = monitor->poll();
    if(!monitorElement) return false;
    userPoll = true;
    pvaClientData->setData(monitorElement);
   return true;
}

bool PvaClientMonitor::waitEvent(double secondsToWait)
{
    if(PvaClient::getDebug()) cout << "PvaClientMonitor::waitEvent\n";
    if(connectState!=monitorStarted) throw std::runtime_error("PvaClientMonitor::waitEvent illegal state");
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
    if(PvaClient::getDebug()) cout << "PvaClientMonitor::releaseEvent\n";
    if(connectState!=monitorStarted) throw std::runtime_error(
         "PvaClientMonitor::poll illegal state");
    if(!userPoll) throw std::runtime_error("PvaClientMonitor::releaseEvent did not call poll");
    userPoll = false;
    monitor->release(monitorElement);
}

PvaClientMonitorDataPtr PvaClientMonitor::getData()
{
    if(PvaClient::getDebug()) cout << "PvaClientMonitor::getData\n";
    checkMonitorState();
    return pvaClientData;
}


}}

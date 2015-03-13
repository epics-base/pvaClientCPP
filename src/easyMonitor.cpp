/* easyMonitor.cpp */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvData is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
/**
 * @author mrk
 * @date 2015.03
 */
#define epicsExportSharedSymbols

#include <sstream>
#include <pv/event.h>
#include <pv/easyPVA.h>
#include <pv/bitSetUtil.h>

using std::tr1::static_pointer_cast;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace std;

namespace epics { namespace easyPVA {


class ChannelMonitorRequester : public MonitorRequester
{
    EasyMonitor * easyMonitor;
public:
    ChannelMonitorRequester(EasyMonitor * easyMonitor)
    : easyMonitor(easyMonitor) {}
    string getRequesterName()
    {return easyMonitor->getRequesterName();}
    void message(string const & message,MessageType messageType)
    {easyMonitor->message(message,messageType);}
    void monitorConnect(
        const Status& status,
        Monitor::shared_pointer const & monitor,
        StructureConstPtr const & structure)
    {easyMonitor->monitorConnect(status,monitor,structure);}
    void monitorEvent(MonitorPtr const & monitor)
    {
         easyMonitor->monitorEvent(monitor);
    }
    void unlisten(MonitorPtr const & monitor)
    {easyMonitor->unlisten();}
};

EasyMonitor::EasyMonitor(
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
  userPoll(false),
  userWait(false)
{
}

EasyMonitor::~EasyMonitor()
{
    destroy();
}

void EasyMonitor::checkMonitorState()
{
    if(isDestroyed) throw std::runtime_error("easyMonitor was destroyed");
    if(connectState==connectIdle) connect();
    if(connectState==connected) start();
}

// from MonitorRequester
string EasyMonitor::getRequesterName()
{
     EasyPVAPtr yyy = easyPVA.lock();
     if(!yyy) throw std::runtime_error("easyPVA was destroyed");
     return yyy->getRequesterName();
}

void EasyMonitor::message(string const & message,MessageType messageType)
{
    if(isDestroyed) throw std::runtime_error("easyMonitor was destroyed");
    EasyPVAPtr yyy = easyPVA.lock();
    if(!yyy) throw std::runtime_error("easyPVA was destroyed");
    yyy->message(message, messageType);
}

void EasyMonitor::monitorConnect(
    const Status& status,
    Monitor::shared_pointer const & monitor,
    StructureConstPtr const & structure)
{
    if(isDestroyed) throw std::runtime_error("easyMonitor was destroyed");
    connectStatus = status;
    this->monitor = monitor;
    if(status.isOK()) {
        easyData = EasyMonitorData::create(structure);
        easyData->setMessagePrefix(channel->getChannelName());
    }
    waitForConnect.signal();
    
}

void EasyMonitor::monitorEvent(MonitorPtr const & monitor)
{
    EasyMonitorRequesterPtr req = easyMonitorRequester.lock();
    if(req) req->event(getPtrSelf());
    if(userWait) waitForEvent.signal();
}

void EasyMonitor::unlisten()
{
    destroy();
}

// from EasyMonitor
void EasyMonitor::destroy()
{
    {
        Lock xx(mutex);
        if(isDestroyed) return;
        isDestroyed = true;
    }
    if(monitor) monitor->destroy();
    monitor.reset();
}

void EasyMonitor::connect()
{
    if(isDestroyed) throw std::runtime_error("easyMonitor was destroyed");
    issueConnect();
    Status status = waitConnect();
    if(status.isOK()) return;
    stringstream ss;
    ss << "channel " << channel->getChannelName() << " EasyMonitor::connect " << status.getMessage();
    throw std::runtime_error(ss.str());
}

void EasyMonitor::issueConnect()
{
    if(isDestroyed) throw std::runtime_error("easyMonitor was destroyed");
    if(connectState!=connectIdle) {
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " easyMonitor already connected ";
        throw std::runtime_error(ss.str());
    }
    monitorRequester = ChannelMonitorRequester::shared_pointer(new ChannelMonitorRequester(this));
    connectState = connectActive;
    monitor = channel->createMonitor(monitorRequester,pvRequest);
}

Status EasyMonitor::waitConnect()
{
    if(isDestroyed) throw std::runtime_error("easyMonitor was destroyed");
    if(connectState!=connectActive) {
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " easyMonitor illegal connect state ";
        throw std::runtime_error(ss.str());
    }
    waitForConnect.wait();
    if(connectStatus.isOK()){
        connectState = connected;
        return Status::Ok;
    }
    connectState = connectIdle;
    return Status(Status::STATUSTYPE_ERROR,connectStatus.getMessage());
}

void EasyMonitor::setRequester(EasyMonitorRequesterPtr const & easyMonitorrRequester)
{
    this->easyMonitorRequester = easyMonitorrRequester;
}

void EasyMonitor::start()
{
    if(isDestroyed) throw std::runtime_error("easyMonitor was destroyed");
    if(connectState==monitorStarted) return;
    if(connectState==connectIdle) connect();
    if(connectState!=connected) throw std::runtime_error("EasyMonitor::start illegal state");
    connectState = monitorStarted;
    monitor->start();
}


void EasyMonitor::stop()
{
    if(isDestroyed) throw std::runtime_error("easyMonitor was destroyed");
    if(connectState!=monitorStarted) return;
    connectState = connected;
    monitor->stop();
}

bool EasyMonitor::poll()
{
    checkMonitorState();
    if(connectState!=monitorStarted) throw std::runtime_error("EasyMonitor::poll illegal state");
    if(userPoll) throw std::runtime_error("EasyMonitor::poll did not release last");
    monitorElement = monitor->poll();
    if(!monitorElement) return false;
    userPoll = true;
    easyData->setData(monitorElement);
   return true;
}

bool EasyMonitor::waitEvent(double secondsToWait)
{
    if(isDestroyed) throw std::runtime_error("easyMonitor was destroyed");
    if(connectState!=monitorStarted) throw std::runtime_error("EasyMonitor::poll illegal state");
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

void EasyMonitor::releaseEvent()
{
    if(isDestroyed) throw std::runtime_error("easyMonitor was destroyed");
    if(connectState!=monitorStarted) throw std::runtime_error("EasyMonitor::poll illegal state");
    if(!userPoll) throw std::runtime_error("EasyMonitor::releaseEvent did not call poll");
    userPoll = false;
    monitor->release(monitorElement);
}

EasyMonitorDataPtr EasyMonitor::getData()
{
    checkMonitorState();
    return easyData;
}

EasyMonitorPtr EasyMonitor::create(
        EasyPVAPtr const &pva,
        EasyChannelPtr const & easyChannel,
        Channel::shared_pointer const & channel,
        PVStructurePtr const &pvRequest)
{
    EasyMonitorPtr epv(new EasyMonitor(pva,easyChannel,channel,pvRequest));
    return epv;
}

}}

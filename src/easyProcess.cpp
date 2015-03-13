/* easyProcess.cpp */
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
#include <pv/easyPVA.h>

using std::tr1::static_pointer_cast;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace std;

namespace epics { namespace easyPVA {


class ChannelProcessRequesterImpl : public ChannelProcessRequester
{
    EasyProcess * easyProcess;
public:
    ChannelProcessRequesterImpl(EasyProcess * easyProcess)
    : easyProcess(easyProcess) {}
    string getRequesterName()
    {return easyProcess->getRequesterName();}
    void message(string const & message,MessageType messageType)
    {easyProcess->message(message,messageType);}
    void channelProcessConnect(
        const Status& status,
        ChannelProcess::shared_pointer const & channelProcess)
    {easyProcess->channelProcessConnect(status,channelProcess);}
    void processDone(
        const Status& status,
        ChannelProcess::shared_pointer const & channelProcess)
    {easyProcess->processDone(status,channelProcess);}
};

EasyProcess::EasyProcess(
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
  processState(processIdle)
{
}

EasyProcess::~EasyProcess()
{
    destroy();
}

void EasyProcess::checkProcessState()
{
    if(isDestroyed) throw std::runtime_error("easyProcess was destroyed");
    if(connectState==connectIdle) connect();
    if(processState==processIdle) process();
}

// from ChannelProcessRequester
string EasyProcess::getRequesterName()
{
     EasyPVAPtr yyy = easyPVA.lock();
     if(!yyy) throw std::runtime_error("easyPVA was destroyed");
     return yyy->getRequesterName();
}

void EasyProcess::message(string const & message,MessageType messageType)
{
    if(isDestroyed) throw std::runtime_error("easyProcess was destroyed");
    EasyPVAPtr yyy = easyPVA.lock();
    if(!yyy) throw std::runtime_error("easyPVA was destroyed");
    yyy->message(message, messageType);
}

void EasyProcess::channelProcessConnect(
    const Status& status,
    ChannelProcess::shared_pointer const & channelProcess)
{
    if(isDestroyed) throw std::runtime_error("easyProcess was destroyed");
    channelProcessConnectStatus = status;
    this->channelProcess = channelProcess;
    waitForConnect.signal();
    
}

void EasyProcess::processDone(
    const Status& status,
    ChannelProcess::shared_pointer const & channelProcess)
{
    if(isDestroyed) throw std::runtime_error("easyProcess was destroyed");
    channelProcessStatus = status;
    waitForProcess.signal();
}


// from EasyProcess
void EasyProcess::destroy()
{
    {
        Lock xx(mutex);
        if(isDestroyed) return;
        isDestroyed = true;
    }
    if(channelProcess) channelProcess->destroy();
    channelProcess.reset();
}

void EasyProcess::connect()
{
    if(isDestroyed) throw std::runtime_error("easyProcess was destroyed");
    issueConnect();
    Status status = waitConnect();
    if(status.isOK()) return;
    stringstream ss;
    ss << "channel " << channel->getChannelName() << " EasyProcess::connect " << status.getMessage();
    throw std::runtime_error(ss.str());
}

void EasyProcess::issueConnect()
{
    if(isDestroyed) throw std::runtime_error("easyProcess was destroyed");
    if(connectState!=connectIdle) {
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " easyProcess already connected ";
        throw std::runtime_error(ss.str());
    }
    processRequester = ChannelProcessRequester::shared_pointer(new ChannelProcessRequesterImpl(this));
    connectState = connectActive;
    channelProcess = channel->createChannelProcess(processRequester,pvRequest);
}

Status EasyProcess::waitConnect()
{
    if(isDestroyed) throw std::runtime_error("easyProcess was destroyed");
    if(connectState!=connectActive) {
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " easyProcess illegal connect state ";
        throw std::runtime_error(ss.str());
    }
    waitForConnect.wait();
    if(channelProcessConnectStatus.isOK()){
        connectState = connected;
        return Status::Ok;
    }
    connectState = connectIdle;
    return Status(Status::STATUSTYPE_ERROR,channelProcessConnectStatus.getMessage());
}

void EasyProcess::process()
{
    if(isDestroyed) throw std::runtime_error("easyProcess was destroyed");
    issueProcess();
    Status status = waitProcess();
    if(status.isOK()) return;
    stringstream ss;
    ss << "channel " << channel->getChannelName() << " EasyProcess::process " << status.getMessage();
    throw std::runtime_error(ss.str());
}

void EasyProcess::issueProcess()
{
    if(isDestroyed) throw std::runtime_error("easyProcess was destroyed");
    if(connectState==connectIdle) connect();
    if(processState!=processIdle) {
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " EasyProcess::issueProcess process aleady active ";
        throw std::runtime_error(ss.str());
    }
    processState = processActive;
    channelProcess->process();
}

Status EasyProcess::waitProcess()
{
    if(isDestroyed) throw std::runtime_error("easyProcess was destroyed");
    if(processState!=processActive){
        stringstream ss;
        ss << "channel " << channel->getChannelName() << " EasyProcess::waitProcess llegal process state";
        throw std::runtime_error(ss.str());
    }
    waitForProcess.wait();
    processState = processIdle;
    if(channelProcessStatus.isOK()) {
        return Status::Ok;
    }
    return Status(Status::STATUSTYPE_ERROR,channelProcessStatus.getMessage());
}

EasyProcessPtr EasyProcess::create(
        EasyPVAPtr const &pva,
        EasyChannelPtr const & easyChannel,
        Channel::shared_pointer const & channel,
        PVStructurePtr const &pvRequest)
{
    EasyProcessPtr epv(new EasyProcess(pva,easyChannel,channel,pvRequest));
    return epv;
}

}}

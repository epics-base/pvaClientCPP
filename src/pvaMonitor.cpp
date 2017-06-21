/* pvaClientMonitor.cpp */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvData is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
/**
 * @author mrk
 * @date 2017.06
 */


#define epicsExportSharedSymbols

#include <pv/pvaClient.h>

using std::tr1::static_pointer_cast;
using std::tr1::dynamic_pointer_cast;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace std;

namespace epics { namespace pvaClient {

ExecutorPtr PvaMonitor::executor(new Executor("pvaMonitor",middlePriority));


PvaMonitorPtr PvaMonitor::create(
        PvaClientPtr const &pvaClient,
        std::string const & channelName,
        std::string const & providerName,
        std::string const & request,
        PvaClientChannelStateChangeRequesterPtr const & stateChangeRequester,
        PvaClientMonitorRequesterPtr const & monitorRequester)
{
    PvaMonitorPtr epv(new PvaMonitor(pvaClient,channelName,providerName,request,
        stateChangeRequester,monitorRequester));
    epv->init();
    return epv;
}

PvaMonitor::PvaMonitor(
        PvaClientPtr const &pvaClient,
        std::string const & channelName,
        std::string const & providerName,
        std::string const & request,
        PvaClientChannelStateChangeRequesterPtr const & stateChangeRequester,
        PvaClientMonitorRequesterPtr const & monitorRequester)
: pvaClient(pvaClient),
  channelName(channelName),
  providerName(providerName),
  request(request),
  stateChangeRequester(stateChangeRequester),
  monitorRequester(monitorRequester)
{
    if(PvaClient::getDebug()) {
         cout<< "PvaMonitor::PvaMonitor()"
             << " channelName " << channelName 
             << endl;
    }
}

void PvaMonitor::init()
{
    PvaClientPtr client(pvaClient.lock());
    if(!client) throw std::runtime_error("pvaClient was destroyed");
    pvaClientChannel = client->createChannel(channelName,providerName);
    pvaClientChannel->setStateChangeRequester(shared_from_this());
    pvaClientChannel->issueConnect();
}

PvaMonitor::~PvaMonitor()
{
    if(PvaClient::getDebug()) cout<< "PvaMonitor::~PvaMonitor\n";
    pvaClientChannel.reset();
    pvaClientMonitor.reset();
}

PvaClientChannelPtr PvaMonitor::getPvaClientChannel()
{
    return pvaClientChannel;
}

PvaClientMonitorPtr PvaMonitor::getPvaClientMonitor()
{
    return pvaClientMonitor;
}

void PvaMonitor::start()
{
    if(PvaClient::getDebug()) cout<< "PvaMonitor::start()\n";
    if(!pvaClientMonitor) {
        PvaClientPtr client(pvaClient.lock());
        client->message("PvaMonitor::start but not connected",MessageType::errorMessage);
        return;
    }
    pvaClientMonitor->start();
}

void PvaMonitor::start(const string & request)
{
    if(PvaClient::getDebug()) cout<< "PvaMonitor::start(request)\n";
    if(!pvaClientChannel->getChannel()->isConnected()) {
        PvaClientPtr client(pvaClient.lock());
        client->message("PvaMonitor::start(request) but not connected",MessageType::errorMessage);
        return;
    }
    pvaClientMonitor.reset();
    pvaClientMonitor = pvaClientChannel->monitor(request,shared_from_this());
}
    
void PvaMonitor::stop()
{
    if(PvaClient::getDebug()) cout<< "PvaMonitor::stop()\n";
    if(!pvaClientMonitor) {
        PvaClientPtr client(pvaClient.lock());
        client->message("PvaMonitor::start but not connected",MessageType::errorMessage);
        return;
    }
    pvaClientMonitor->stop();
}

void PvaMonitor::channelStateChange(PvaClientChannelPtr const & channel, bool isConnected)
{
    if(PvaClient::getDebug()) cout<< "PvaMonitor::channelStateChange isConnected " << (isConnected ? "true" : "false") << endl;
    if(isConnected&&!pvaClientMonitor)
    {
         if(PvaClient::getDebug()) cout<< "PvaMonitor::channelStateChange calling executor.execute\n";
         executor->execute(shared_from_this());
        }
        if(stateChangeRequester) stateChangeRequester->channelStateChange(channel,isConnected);
    }

void PvaMonitor::event(PvaClientMonitorPtr const & monitor)
{
   if(monitorRequester) monitorRequester->event(monitor);
}

void PvaMonitor::command()
{
    if(PvaClient::getDebug()) cout<< "PvaMonitor::command\n";
    pvaClientMonitor = pvaClientChannel->monitor(request,shared_from_this());
}

}}

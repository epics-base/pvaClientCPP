/* pvaClient.cpp */
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
#include <pv/pvaClient.h>
#include <pv/createRequest.h>
#include <pv/clientFactory.h>
#include <pv/caProvider.h>

using std::tr1::static_pointer_cast;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvAccess::ca;
using namespace std;

namespace epics { namespace pvaClient { 

static FieldCreatePtr fieldCreate = getFieldCreate(); 
static const string pvaClientName = "pvaClient";
static const string defaultProvider = "pva";
static UnionConstPtr variantUnion = fieldCreate->createVariantUnion();

namespace pvaClientPvt {

    static size_t numberPvaClient = 0;
    static bool firstTime = true;
    static Mutex mutex;
    
    class StartStopClientFactory {
    public:
        static void PvaClientBeingConstructed()
        {
            bool saveFirst = false;
            { 
                 Lock xx(mutex);
                 ++numberPvaClient;
                 saveFirst = firstTime;
                 firstTime = false;
            }
            if(saveFirst) {
                ClientFactory::start();
                CAClientFactory::start();
            }
        }
    
        static void PvaClientBeingDestroyed() {
            size_t numLeft = 0;
            {
                 Lock xx(mutex);
                 --numberPvaClient;
                  numLeft = numberPvaClient;
            }
            if(numLeft<=0) {
                ClientFactory::stop();
                CAClientFactory::stop();
            }
        }
    };

} // namespace pvaClientPvt

class PvaClientChannelCache
{
public:
    PvaClientChannelCache(){}
    ~PvaClientChannelCache(){
         destroy();
     }
    void destroy() {
       pvaClientChannelMap.clear();
    }
    PvaClientChannelPtr getChannel(string const & channelName);
    void addChannel(PvaClientChannelPtr const & pvaClientChannel);
    void removeChannel(string const & channelName);
private:
    map<string,PvaClientChannelPtr> pvaClientChannelMap;
};
   
PvaClientChannelPtr PvaClientChannelCache::getChannel(string const & channelName)
{
    map<string,PvaClientChannelPtr>::iterator iter = pvaClientChannelMap.find(channelName);
    if(iter!=pvaClientChannelMap.end()) return iter->second;
    return PvaClientChannelPtr();
}

void PvaClientChannelCache::addChannel(PvaClientChannelPtr const & pvaClientChannel)
{
     pvaClientChannelMap.insert(std::pair<string,PvaClientChannelPtr>(
         pvaClientChannel->getChannelName(),pvaClientChannel));
}

void PvaClientChannelCache::removeChannel(string const & channelName)
{
    map<string,PvaClientChannelPtr>::iterator iter = pvaClientChannelMap.find(channelName);
    if(iter!=pvaClientChannelMap.end()) pvaClientChannelMap.erase(iter);
}

using namespace epics::pvaClient::pvaClientPvt;

PvaClientPtr PvaClient::create()
{
    PvaClientPtr xx(new PvaClient());
    StartStopClientFactory::PvaClientBeingConstructed();
    return xx;
}

PVStructurePtr PvaClient::createRequest(string const &request)
{
    CreateRequest::shared_pointer createRequest = CreateRequest::create();
    PVStructurePtr pvRequest = createRequest->createRequest(request);
    if(!pvRequest) {
        throw std::invalid_argument("invalid pvRequest: " + createRequest->getMessage());
    }
    return pvRequest;
}

PvaClient::PvaClient()
:   pvaClientChannelCache(new PvaClientChannelCache()),
    isDestroyed(false)
{
}

PvaClient::~PvaClient() {
    destroy();
}

void PvaClient::destroy()
{
    {
        Lock xx(mutex);
        if(isDestroyed) return;
        isDestroyed = true;
    }
    pvaClientChannelCache.reset();
    channelList.clear();
    multiChannelList.clear();
    StartStopClientFactory::PvaClientBeingDestroyed();
}

string PvaClient:: getRequesterName()
{
    static string name("pvaClient");
    return name;
}

void  PvaClient::message(
        string const & message,
        MessageType messageType)
{
    cout << getMessageTypeName(messageType) << " " << message << endl;
}

PvaClientChannelPtr PvaClient::channel(
        std::string const & channelName,
        std::string const & providerName,
        double timeOut)
{
    PvaClientChannelPtr pvaClientChannel = pvaClientChannelCache->getChannel(channelName);
    if(pvaClientChannel) return pvaClientChannel;
    pvaClientChannel = createChannel(channelName,providerName);
    pvaClientChannel->connect(timeOut);
    pvaClientChannelCache->addChannel(pvaClientChannel);
    return pvaClientChannel;
}

PvaClientChannelPtr PvaClient::createChannel(string const & channelName)
{
     return PvaClientChannel::create(getPtrSelf(),channelName);
}

PvaClientChannelPtr PvaClient::createChannel(string const & channelName, string const & providerName)
{
     return PvaClientChannel::create(getPtrSelf(),channelName,providerName);
}

PvaClientMultiChannelPtr PvaClient::createMultiChannel(
    epics::pvData::PVStringArrayPtr const & channelNames)
{
    return createMultiChannel(channelNames,"pvaClient");
}

PvaClientMultiChannelPtr PvaClient::createMultiChannel(
    epics::pvData::PVStringArrayPtr const & channelNames,
    std::string const & providerName)
{
    return PvaClientMultiChannel::create(getPtrSelf(),channelNames,providerName);
}

}}


/* easyPVA.cpp */
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
#include <pv/easyPVA.h>
#include <pv/createRequest.h>
#include <pv/clientFactory.h>

using std::tr1::static_pointer_cast;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace std;

namespace epics { namespace easyPVA { 

static FieldCreatePtr fieldCreate = getFieldCreate(); 
static const string easyPVAName = "easyPVA";
static const string defaultProvider = "pva";
static UnionConstPtr variantUnion = fieldCreate->createVariantUnion();

namespace easyPVAPvt {

    static size_t numberEasyPVA = 0;
    static bool firstTime = true;
    static Mutex mutex;
    
    class StartStopClientFactory {
    public:
        static void EasyPVABeingConstructed()
        {
            bool saveFirst = false;
            { 
                 Lock xx(mutex);
                 ++numberEasyPVA;
                 saveFirst = firstTime;
                 firstTime = false;
            }
            if(saveFirst) ClientFactory::start();
        }
    
        static void EasyPVABeingDestroyed() {
            size_t numLeft = 0;
            {
                 Lock xx(mutex);
                 --numberEasyPVA;
                  numLeft = numberEasyPVA;
            }
            if(numLeft<=0) ClientFactory::stop();
        }
    };

} // namespace easyPVAPvt

class EasyChannelCache
{
public:
    EasyChannelCache(){}
    ~EasyChannelCache(){
         destroy();
     }
    void destroy() {
       easyChannelMap.clear();
    }
    EasyChannelPtr getChannel(string const & channelName);
    void addChannel(EasyChannelPtr const & easyChannel);
    void removeChannel(string const & channelName);
private:
    map<string,EasyChannelPtr> easyChannelMap;
};
   
EasyChannelPtr EasyChannelCache::getChannel(string const & channelName)
{
    map<string,EasyChannelPtr>::iterator iter = easyChannelMap.find(channelName);
    if(iter!=easyChannelMap.end()) return iter->second;
    return EasyChannelPtr();
}

void EasyChannelCache::addChannel(EasyChannelPtr const & easyChannel)
{
     easyChannelMap.insert(std::pair<string,EasyChannelPtr>(
         easyChannel->getChannelName(),easyChannel));
}

void EasyChannelCache::removeChannel(string const & channelName)
{
    map<string,EasyChannelPtr>::iterator iter = easyChannelMap.find(channelName);
    if(iter!=easyChannelMap.end()) easyChannelMap.erase(iter);
}

using namespace epics::easyPVA::easyPVAPvt;

EasyPVAPtr EasyPVA::create()
{
    EasyPVAPtr xx(new EasyPVA());
    StartStopClientFactory::EasyPVABeingConstructed();
    return xx;
}

PVStructurePtr EasyPVA::createRequest(string const &request)
{
    CreateRequest::shared_pointer createRequest = CreateRequest::create();
    PVStructurePtr pvRequest = createRequest->createRequest(request);
    if(!pvRequest) {
        throw std::invalid_argument("invalid pvRequest: " + createRequest->getMessage());
    }
    return pvRequest;
}

EasyPVA::EasyPVA()
:   easyChannelCache(new EasyChannelCache()),
    isDestroyed(false)
{
}

EasyPVA::~EasyPVA() {
    destroy();
}

void EasyPVA::destroy()
{
    {
        Lock xx(mutex);
        if(isDestroyed) return;
        isDestroyed = true;
    }
    easyChannelCache.reset();
    channelList.clear();
    multiChannelList.clear();
    StartStopClientFactory::EasyPVABeingDestroyed();
}

string EasyPVA:: getRequesterName()
{
    static string name("easyPVA");
    return name;
}

void  EasyPVA::message(
        string const & message,
        MessageType messageType)
{
    cout << getMessageTypeName(messageType) << " " << message << endl;
}

EasyChannelPtr EasyPVA::channel(
        std::string const & channelName,
        std::string const & providerName,
        double timeOut)
{
    EasyChannelPtr easyChannel = easyChannelCache->getChannel(channelName);
    if(easyChannel) return easyChannel;
    easyChannel = createChannel(channelName,providerName);
    easyChannel->connect(timeOut);
    easyChannelCache->addChannel(easyChannel);
    return easyChannel;
}

EasyChannelPtr EasyPVA::createChannel(string const & channelName)
{
     return EasyChannel::create(getPtrSelf(),channelName);
}

EasyChannelPtr EasyPVA::createChannel(string const & channelName, string const & providerName)
{
     return EasyChannel::create(getPtrSelf(),channelName,providerName);
}

EasyMultiChannelPtr EasyPVA::createMultiChannel(
    epics::pvData::PVStringArrayPtr const & channelNames)
{
    return createMultiChannel(channelNames,"pva");
}

EasyMultiChannelPtr EasyPVA::createMultiChannel(
    epics::pvData::PVStringArrayPtr const & channelNames,
    std::string const & providerName)
{
    return EasyMultiChannel::create(getPtrSelf(),channelNames,providerName);
}

}}


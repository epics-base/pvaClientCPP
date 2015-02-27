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
#include <pv/easyPVA.h>
#include <pv/createRequest.h>

using std::tr1::static_pointer_cast;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace std;

namespace epics { namespace easyPVA { 

static FieldCreatePtr fieldCreate = getFieldCreate(); 
static const string easyPVAName = "easyPVA";
static const string defaultProvider = "pva";
static UnionConstPtr variantUnion = fieldCreate->createVariantUnion();

EasyPVAPtr EasyPVA::create()
{
    EasyPVAPtr xx(new EasyPVA());
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
: isDestroyed(false)
{
}

EasyPVA::~EasyPVA() {destroy();}

void EasyPVA::destroy()
{
    {
        Lock xx(mutex);
        if(isDestroyed) return;
        isDestroyed = true;
    }
    std::list<EasyChannelPtr>::iterator channelIter;
    while(true) {
        channelIter = channelList.begin();
        if(channelIter==channelList.end()) break;
        channelList.erase(channelIter);
        (*channelIter)->destroy();
    }
    std::list<EasyMultiChannelPtr>::iterator multiChannelIter;
#ifdef NOTDONE
    while(true) {
        multiChannelIter = multiChannelList.begin();
        if(multiChannelIter==multiChannelList.end()) break;
        multiChannelList.erase(multiChannelIter);
        (*multiChannelIter)->destroy();
    }
#endif

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

EasyPVStructurePtr EasyPVA::createEasyPVStructure()
{
    return EasyPVStructureFactory::createEasyPVStructure();
}

EasyChannelPtr EasyPVA::createChannel(string const & channelName)
{
     return EasyChannelFactory::createEasyChannel(getPtrSelf(),channelName);
}

EasyChannelPtr EasyPVA::createChannel(string const & channelName, string const & providerName)
{
     return EasyChannelFactory::createEasyChannel(getPtrSelf(),channelName,providerName);
}

}}


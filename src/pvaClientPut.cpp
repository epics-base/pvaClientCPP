/* pvaClientPut.cpp */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvData is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
/**
 * @author mrk
 * @date 2015.02
 */

#include <pv/event.h>

#define epicsExportSharedSymbols

#include <pv/pvaClient.h>

using std::tr1::static_pointer_cast;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace std;

namespace epics { namespace pvaClient {

class ChannelPutRequesterImpl : public ChannelPutRequester
{
    PvaClientPut::weak_pointer pvaClientPut;
    PvaClient::weak_pointer pvaClient;
public:
    ChannelPutRequesterImpl(
        PvaClientPutPtr const & pvaClientPut,
        PvaClientPtr const &pvaClient)
    : pvaClientPut(pvaClientPut),
      pvaClient(pvaClient)
    {}
    virtual ~ChannelPutRequesterImpl() {
        if(PvaClient::getDebug()) std::cout << "~ChannelPutRequesterImpl" << std::endl;
    }

    virtual std::string getRequesterName() {
        PvaClientPutPtr clientPut(pvaClientPut.lock());
        if(!clientPut) return string("clientPut is null");
        return clientPut->getRequesterName();
    }

    virtual void message(std::string const & message, epics::pvData::MessageType messageType) {
        PvaClientPutPtr clientPut(pvaClientPut.lock());
        if(!clientPut) return;
        clientPut->message(message,messageType);
    }

    virtual void channelPutConnect(
        const Status& status,
        ChannelPut::shared_pointer const & channelPut,
        Structure::const_shared_pointer const & structure)
    {
        PvaClientPutPtr clientPut(pvaClientPut.lock());
        if(!clientPut) return;
        clientPut->channelPutConnect(status,channelPut,structure);  
    }

    virtual void getDone(
        const Status& status,
        ChannelPut::shared_pointer const & channelPut,
        PVStructurePtr const & pvStructure,
        BitSet::shared_pointer const & bitSet)
    {
        PvaClientPutPtr clientPut(pvaClientPut.lock());
        if(!clientPut) return;
        clientPut->getDone(status,channelPut,pvStructure,bitSet);
    }

        virtual void putDone(
        const Status& status,
        ChannelPut::shared_pointer const & channelPut)
    {
        PvaClientPutPtr clientPut(pvaClientPut.lock());
        if(!clientPut) return;
        clientPut->putDone(status,channelPut);
    }
};

PvaClientPutPtr PvaClientPut::create(
        PvaClientPtr const &pvaClient,
        PvaClientChannelPtr const & pvaClientChannel,
        PVStructurePtr const &pvRequest)
{
    PvaClientPutPtr clientPut(new PvaClientPut(pvaClient,pvaClientChannel,pvRequest));
    clientPut->channelPutRequester = ChannelPutRequesterImplPtr(
        new ChannelPutRequesterImpl(clientPut,pvaClient));
    return clientPut;
}

PvaClientPutPtr PvaClientPut::create(
        PvaClientPtr const &pvaClient,
        std::string const & channelName,
        std::string const & providerName,
        std::string const & request,
        PvaClientChannelStateChangeRequesterPtr const & stateChangeRequester,
        PvaClientPutRequesterPtr const & putRequester)
{
    if(PvaClient::getDebug()) {
         cout<< "PvaClientPut::create(pvaClient,channelName,providerName,request,stateChangeRequester,putRequester)\n"
             << " channelName " <<  channelName
             << " providerName " <<  providerName
             << " request " << request
             << endl;
    }
    CreateRequest::shared_pointer createRequest(CreateRequest::create());
    PVStructurePtr pvRequest(createRequest->createRequest(request));
    if(!pvRequest) throw std::runtime_error(createRequest->getMessage());
    PvaClientChannelPtr pvaClientChannel = pvaClient->createChannel(channelName,providerName);
    PvaClientPutPtr clientPut(new PvaClientPut(pvaClient,pvaClientChannel,pvRequest));
    clientPut->channelPutRequester = ChannelPutRequesterImplPtr(
        new ChannelPutRequesterImpl(clientPut,pvaClient));
    if(stateChangeRequester) clientPut->pvaClientChannelStateChangeRequester = stateChangeRequester;
    if(putRequester) clientPut->pvaClientPutRequester = putRequester;
    pvaClientChannel->setStateChangeRequester(clientPut);
    pvaClientChannel->issueConnect();
    return clientPut;
}


PvaClientPut::PvaClientPut(
        PvaClientPtr const &pvaClient,
        PvaClientChannelPtr const & pvaClientChannel,
        PVStructurePtr const &pvRequest)
: pvaClient(pvaClient),
  pvaClientChannel(pvaClientChannel),
  pvRequest(pvRequest),
  connectState(connectIdle),
  putState(putIdle)
{
    if(PvaClient::getDebug()) {
         cout<< "PvaClientPut::PvaClientPut"
             << " channelName " <<  pvaClientChannel->getChannel()->getChannelName()
             << endl;
    }
}

PvaClientPut::~PvaClientPut()
{
    if(PvaClient::getDebug()) {
        cout<< "PvaClientPut::~PvaClientPut"
           << " channelName " <<  pvaClientChannel->getChannel()->getChannelName()
           << endl;
    }
    if(channelPut) channelPut->destroy();
}

void PvaClientPut::channelStateChange(PvaClientChannelPtr const & pvaClientChannel, bool isConnected)
{
    if(PvaClient::getDebug()) {
           cout<< "PvaClientPut::channelStateChange"
               << " channelName " << pvaClientChannel->getChannel()->getChannelName()
               << " isConnected " << (isConnected ? "true" : "false")
               << endl;
    }
    if(isConnected&&!channelPut)
    {
        connectState = connectActive;
        channelPut = pvaClientChannel->getChannel()->createChannelPut(channelPutRequester,pvRequest);
    }
    PvaClientChannelStateChangeRequesterPtr req(pvaClientChannelStateChangeRequester.lock());
    if(req) {
          req->channelStateChange(pvaClientChannel,isConnected);
    }
}

void PvaClientPut::checkPutState()
{
    if(PvaClient::getDebug()) {
        cout << "PvaClientPut::checkPutState"
           << " channelName " << pvaClientChannel->getChannel()->getChannelName()
           << endl;
    }
    if(connectState==connectIdle){
          connect();
          get();
          return;
    }
    if(connectState==connectActive){
        string message = string("channel ") + pvaClientChannel->getChannel()->getChannelName()
            + " "
            + channelPutConnectStatus.getMessage();
        throw std::runtime_error(message);
    }
}

string PvaClientPut::getRequesterName()
{
     PvaClientPtr yyy = pvaClient.lock();
     if(!yyy) return string("PvaClientPut::getRequesterName() PvaClient isDestroyed");
     return yyy->getRequesterName();
}

void PvaClientPut::message(string const & message,MessageType messageType)
{
    PvaClientPtr yyy = pvaClient.lock();
    if(!yyy) return;
    yyy->message(message, messageType);
}

void PvaClientPut::channelPutConnect(
    const Status& status,
    ChannelPut::shared_pointer const & channelPut,
    StructureConstPtr const & structure)
{
    if(PvaClient::getDebug()) {
        cout << "PvaClientPut::channelPutConnect"
           << " channelName " << pvaClientChannel->getChannel()->getChannelName()
           << " status.isOK " << (status.isOK() ? "true" : "false")
           << endl;
    }
    {
        Lock xx(mutex);
        this->channelPut = channelPut;
        if(status.isOK()) {
            channelPutConnectStatus = status;
            connectState = connected;
            pvaClientData = PvaClientPutData::create(structure);
            pvaClientData->setMessagePrefix(channelPut->getChannel()->getChannelName());
        } else {
             stringstream ss;
             ss << pvRequest;
             string message = string("\nPvaClientPut::channelPutConnect)")
               + "\npvRequest\n" + ss.str()
               + "\nerror\n" + status.getMessage();
             channelPutConnectStatus = Status(Status::STATUSTYPE_ERROR,message);
        }
    }
    PvaClientPutRequesterPtr  req(pvaClientPutRequester.lock());
    if(req) {
          req->channelPutConnect(status,shared_from_this());
    }
    waitForConnect.signal();
    
}

void PvaClientPut::getDone(
    const Status& status,
    ChannelPut::shared_pointer const & channelPut,
    PVStructurePtr const & pvStructure,
    BitSetPtr const & bitSet)
{
    if(PvaClient::getDebug()) {
        cout << "PvaClientPut::getDone"
           << " channelName " << pvaClientChannel->getChannel()->getChannelName()
           << " status.isOK " << (status.isOK() ? "true" : "false")
           << endl;
    }
    channelGetPutStatus = status;
    if(status.isOK()) {
        PVStructurePtr pvs = pvaClientData->getPVStructure();
        pvs->copyUnchecked(*pvStructure,*bitSet);
        BitSetPtr bs = pvaClientData->getChangedBitSet();
        bs->clear();
        *bs |= *bitSet;
    }
    PvaClientPutRequesterPtr  req(pvaClientPutRequester.lock());
    if(req) {
          req->getDone(status,shared_from_this());
    }
    waitForGetPut.signal();
}

void PvaClientPut::putDone(
    const Status& status,
    ChannelPut::shared_pointer const & channelPut)
{
    if(PvaClient::getDebug()) {
        cout << "PvaClientPut::putDone"
           << " channelName " << pvaClientChannel->getChannel()->getChannelName()
           << " status.isOK " << (status.isOK() ? "true" : "false")
           << endl;
    }
    channelGetPutStatus = status;
    PvaClientPutRequesterPtr  req(pvaClientPutRequester.lock());
    if(req) {
          req->putDone(status,shared_from_this());
    }
    waitForGetPut.signal();
}

void PvaClientPut::connect()
{
    if(PvaClient::getDebug()) {
        cout << "PvaClientPut::connect"
           << " channelName " << pvaClientChannel->getChannel()->getChannelName()
           << endl;
    }
    issueConnect();
    Status status = waitConnect();
    if(status.isOK()) return;
    string message = string("channel ") 
        + pvaClientChannel->getChannel()->getChannelName()
        + " PvaClientPut::connect "
        + status.getMessage();
    throw std::runtime_error(message);
}

void PvaClientPut::issueConnect()
{
    if(PvaClient::getDebug()) {
        cout << "PvaClientPut::issueConnect"
           << " channelName " << pvaClientChannel->getChannel()->getChannelName()
           << endl;
    }
    if(connectState!=connectIdle) {
        string message = string("channel ") + pvaClientChannel->getChannel()->getChannelName()
            + " pvaClientPut already connected ";
        throw std::runtime_error(message);
    }
    connectState = connectActive;
    channelPutConnectStatus = Status(Status::STATUSTYPE_ERROR, "connect active");
    channelPut = pvaClientChannel->getChannel()->createChannelPut(channelPutRequester,pvRequest);
       
}

Status PvaClientPut::waitConnect()
{
    if(PvaClient::getDebug()) {
        cout << "PvaClientPut::waitConnect"
           << " channelName " << pvaClientChannel->getChannel()->getChannelName()
           << endl;
    }
    {
        Lock xx(mutex);
        if(connectState==connected) {
             if(!channelPutConnectStatus.isOK()) connectState = connectIdle;
             return channelPutConnectStatus;
        }
        if(connectState!=connectActive) {
            string message = string("channel ") + pvaClientChannel->getChannel()->getChannelName()
                + " PvaClientPut::waitConnect illegal connect state ";
            throw std::runtime_error(message);
        }
    }
    waitForConnect.wait();
    if(!channelPutConnectStatus.isOK()) connectState = connectIdle;
    return channelPutConnectStatus;
}

void PvaClientPut::get()
{
    issueGet();
    Status status = waitGet();
    if(status.isOK()) return;
    string message = string("channel ") 
        +  pvaClientChannel->getChannel()->getChannelName()
        + " PvaClientPut::get "
        + status.getMessage();
    throw std::runtime_error(message);
}

void PvaClientPut::issueGet()
{
    if(connectState==connectIdle) connect();
    if(putState!=putIdle) {
        string message = string("channel ")
            + pvaClientChannel->getChannel()->getChannelName()
            +  "PvaClientPut::issueGet get or put aleady active ";
        throw std::runtime_error(message);
    }
    putState = getActive;
    channelPut->get();
}

Status PvaClientPut::waitGet()
{
    if(putState!=getActive){
        string message = string("channel ")
            + pvaClientChannel->getChannel()->getChannelName()
            +  " PvaClientPut::waitGet illegal put state";
        throw std::runtime_error(message);
    }
    waitForGetPut.wait();
    putState = putIdle;
    return channelGetPutStatus;
}

void PvaClientPut::put()
{
    issuePut();
    Status status = waitPut();
    if(status.isOK()) return;
    string message = string("channel ")
        + pvaClientChannel->getChannel()->getChannelName()
        + " PvaClientPut::put "
        + status.getMessage();
    throw std::runtime_error(message);
}

void PvaClientPut::issuePut()
{
    if(connectState==connectIdle) connect();
    if(putState!=putIdle) {
         string message = string("channel ")
            + pvaClientChannel->getChannel()->getChannelName()
            +  "PvaClientPut::issueGet get or put aleady active ";
         throw std::runtime_error(message);
    }
    putState = putActive;
    channelPut->put(pvaClientData->getPVStructure(),pvaClientData->getChangedBitSet());
}

Status PvaClientPut::waitPut()
{
    if(putState!=putActive){
         string message = string("channel ")
            + pvaClientChannel->getChannel()->getChannelName()
            +  " PvaClientPut::waitPut illegal put state";
         throw std::runtime_error(message);
    }
    waitForGetPut.wait();
    putState = putIdle;
    if(channelGetPutStatus.isOK()) pvaClientData->getChangedBitSet()->clear();
    return channelGetPutStatus;
}

PvaClientPutDataPtr PvaClientPut::getData()
{
    if(PvaClient::getDebug()) {
           cout<< "PvaClientPut::getData"
               << " channelName " << pvaClientChannel->getChannel()->getChannelName()
               << endl;
    }
    checkPutState();
    return pvaClientData;
}

void PvaClientPut::setRequester(PvaClientPutRequesterPtr const & pvaClientPutRequester)
{
    if(PvaClient::getDebug()) {
        cout << "PvaClientPut::setRequester"
           << " channelName " << pvaClientChannel->getChannel()->getChannelName()
           << endl;
    }
    this->pvaClientPutRequester = pvaClientPutRequester;
}

PvaClientChannelPtr PvaClientPut::getPvaClientChannel()
{
    return pvaClientChannel;
}

}}

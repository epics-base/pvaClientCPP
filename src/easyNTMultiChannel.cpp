/* easyNTMultiChannel.cpp */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvData is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
/**
 * @author mrk
 * @date 2015.03
 */

#include <pv/easyNTMultiChannel.h>

using std::tr1::static_pointer_cast;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::nt;
using namespace std;

namespace epics { namespace easyPVA { 

EasyNTMultiChannelPtr EasyNTMultiChannel::create(
    EasyPVAPtr const & easyPVA,
    PVStringArrayPtr const & channelName,
    StructureConstPtr const &structure,
    double timeout,
    std::string const & providerName)
{
    EasyMultiChannelPtr easyMultiChannel(
        EasyMultiChannel::create(easyPVA,channelName,providerName));
    Status status = easyMultiChannel->connect(timeout,0);
    if(!status.isOK()) throw std::runtime_error(status.getMessage());
    if(!NTMultiChannel::is_a(structure)) throw std::runtime_error("structure is not valid");
    PVStructurePtr pvStructure = getPVDataCreate()->createPVStructure(structure);
    pvStructure->getSubField<PVStringArray>("channelName")->
        replace(easyMultiChannel->getChannelNames()->view());
    pvStructure->getSubField<PVBooleanArray>("isConnected")->
        replace(easyMultiChannel->getIsConnected()->view());
    NTMultiChannelPtr ntMultiChannel(NTMultiChannel::wrap(pvStructure));
    return EasyNTMultiChannelPtr(new EasyNTMultiChannel(easyMultiChannel,ntMultiChannel));
}

EasyNTMultiChannel::EasyNTMultiChannel(
        EasyMultiChannelPtr const &easyMultiChannel,
        NTMultiChannelPtr const &ntMultiChannel)
:
   easyMultiChannel(easyMultiChannel),
   ntMultiChannel(ntMultiChannel),
   pvUnionArray(ntMultiChannel->getPVStructure()->getSubField<PVUnionArray>("value")),
   pvDataCreate(getPVDataCreate())
{}

EasyNTMultiChannel::~EasyNTMultiChannel()
{
}

void EasyNTMultiChannel::createGet()
{
    PVStructurePtr pvStructure = ntMultiChannel->getPVStructure();
    bool getAlarm = false;
    if(pvStructure->getSubField("severity")) getAlarm = true;
    if(pvStructure->getSubField("status")) getAlarm = true;
    if(pvStructure->getSubField("severity")) getAlarm = true;
    bool getTimeStamp = false;
    if(pvStructure->getSubField("secondsPastEpoch")) getTimeStamp = true;
    if(pvStructure->getSubField("nanoseconds")) getTimeStamp = true;
    if(pvStructure->getSubField("userTag")) getTimeStamp = true;
    string request = "value";
    if(getAlarm) request += ",alarm";
    if(getTimeStamp) request += ",timeStamp";
    EasyChannelArrayPtr easyChannelArray = easyMultiChannel->getEasyChannelArray().lock();
    if(!easyChannelArray)  throw std::runtime_error("easyChannelArray is gone");
    shared_vector<const EasyChannelPtr> easyChannels = *easyChannelArray;
    size_t numChannel = easyChannels.size();
    easyGet = std::vector<EasyGetPtr>(numChannel,EasyGetPtr());
    bool allOK = true;
    string message;
    for(size_t i=0; i<numChannel; ++i)
    {
        easyGet[i] = easyChannels[i]->createGet(request);
        easyGet[i]->issueConnect();
    }
    for(size_t i=0; i<numChannel; ++i)
    {
         Status status = easyGet[i]->waitConnect();
         if(!status.isOK()) {
             message = "connect status " + status.getMessage();
             allOK = false;
             break;
         }
    }
    if(!allOK) throw std::runtime_error(message);
    
}

void EasyNTMultiChannel::createPut()
{
    EasyChannelArrayPtr easyChannelArray = easyMultiChannel->getEasyChannelArray().lock();
    if(!easyChannelArray)  throw std::runtime_error("easyChannelArray is gone");
    shared_vector<const EasyChannelPtr> easyChannels = *easyChannelArray;
    size_t numChannel = easyChannels.size();
    easyPut = std::vector<EasyPutPtr>(numChannel,EasyPutPtr());
    bool allOK = true;
    string message;
    for(size_t i=0; i<numChannel; ++i)
    {
        easyPut[i] = easyChannels[i]->createPut("value");
        easyPut[i]->issueConnect();
    }
    for(size_t i=0; i<numChannel; ++i)
    {
         Status status = easyPut[i]->waitConnect();
         if(!status.isOK()) {
             message = "connect status " + status.getMessage();
             allOK = false;
             break;
         }
    }
    if(!allOK) throw std::runtime_error(message);
}

NTMultiChannelPtr EasyNTMultiChannel::get()
{
    if(easyGet.empty()) createGet();
    PVStructurePtr pvStructure = ntMultiChannel->getPVStructure();
    shared_vector<const string> channelNames = easyMultiChannel->getChannelNames()->view();
    size_t numChannel = channelNames.size();
    bool severityExists = false;
    bool statusExists = false;
    bool messageExists = false;
    bool secondsPastEpochExists = false;
    bool nanosecondsExists = false;
    bool userTagExists = false;
    if(pvStructure->getSubField("severity")) {
        severity.resize(numChannel);
        severityExists = true;
    }
    if(pvStructure->getSubField("status")) {
        status.resize(numChannel);
        statusExists = true;
    }
    if(pvStructure->getSubField("message")) {
        message.resize(numChannel);
        messageExists = true;
    }
    if(pvStructure->getSubField("secondsPastEpoch")) {
        secondsPastEpoch.resize(numChannel);
        secondsPastEpochExists = true;
    }
    if(pvStructure->getSubField("nanoseconds")) {
        nanoseconds.resize(numChannel);
        nanosecondsExists = true;
    }
    if(pvStructure->getSubField("userTag")) {
        userTag.resize(numChannel);
        userTagExists = true;
    }
    valueVector.resize(numChannel);
    
    for(size_t i=0; i<numChannel; ++i)
    {
        easyGet[i]->issueGet();
    }
    for(size_t i=0; i<numChannel; ++i)
    {
        Status stat = easyGet[i]->waitGet();
        if(!stat.isOK()) {
            string message = channelNames[i] + " " + stat.getMessage();
            throw std::runtime_error(message);
        }
        PVStructurePtr pvStructure = easyGet[i]->getData()->getPVStructure();
        PVFieldPtr pvField = pvStructure->getSubField("value");
        if(!pvField) {
            string message = channelNames[i] + " no value field";
            throw std::runtime_error(message);
        }
        UnionConstPtr u = pvUnionArray->getUnionArray()->getUnion();
        if(u->isVariant()) {
              PVUnionPtr pvUnion = pvDataCreate->createPVVariantUnion();
              pvUnion->set(pvField);
              valueVector[i] = pvUnion;
        } else {
              PVUnionPtr pvUnion = pvDataCreate->createPVUnion(u);
              pvUnion->set(pvField);
              valueVector[i] = pvUnion;
        }
        pvField = pvStructure->getSubField("alarm");
        if(pvField) {
            if(pvAlarm.attach(pvField)) {
                 pvAlarm.get(alarm);
                 if(severityExists) severity[i] = alarm.getSeverity();
                 if(statusExists) status[i] = alarm.getStatus();
                 if(messageExists) message[i] = alarm.getMessage();
            }
        }
        pvField = pvStructure->getSubField("timeStamp");
        if(pvField) {
            if(pvTimeStamp.attach(pvField)) {
                 pvTimeStamp.get(timeStamp);
                 if(secondsPastEpochExists) secondsPastEpoch[i] =
                     timeStamp.getSecondsPastEpoch();
                 if(nanosecondsExists) nanoseconds[i] =
                     timeStamp.getNanoseconds();
                 if(userTagExists) userTag[i] = timeStamp.getUserTag();
            }
        }
    }
    pvUnionArray->replace(freeze(valueVector));
    if(severityExists) {
         pvStructure->getSubField<PVIntArray>("severity")->replace(
             freeze(severity));
    }
    if(statusExists) {
         pvStructure->getSubField<PVIntArray>("status")->replace(
             freeze(status));
    }
    if(messageExists) {
         pvStructure->getSubField<PVStringArray>("message")->replace(freeze(message));
    }
    if(secondsPastEpochExists) {
         pvStructure->getSubField<PVLongArray>("secondsPastEpoch")->replace(freeze(secondsPastEpoch));
    }
    if(nanosecondsExists) {
         pvStructure->getSubField<PVIntArray>("nanoseconds")->replace(freeze(nanoseconds));
    }
    if(userTagExists) {
         pvStructure->getSubField<PVIntArray>("userTag")->replace(freeze(userTag));
    }
    return ntMultiChannel;
}

void EasyNTMultiChannel::put(NTMultiChannelPtr const &value)
{
    if(easyPut.empty()) createPut();
    shared_vector<const string> channelNames = easyMultiChannel->getChannelNames()->view();
    size_t numChannel = channelNames.size();
    PVUnionArrayPtr pvValue = value->getPVStructure()->
        getSubField<PVUnionArray>("value");
    shared_vector<const PVUnionPtr> valueVector = pvValue->view();
    for(size_t i=0; i<numChannel; ++i)
    {
        try {
            PVFieldPtr pvFrom = valueVector[i]->get();
            PVFieldPtr pvTo = easyPut[i]->getData()->getValue();
            pvTo->copy(*pvFrom);
            easyPut[i]->issuePut();
        } catch (std::exception e) {
            string message = channelNames[i] + e.what();
            throw std::runtime_error(message);
        }
    }
    for(size_t i=0; i<numChannel; ++i)
    {
        Status status = easyPut[i]->waitPut();
        if(!status.isOK()) {
            string message = channelNames[i] + " " + status.getMessage();
            throw std::runtime_error(message);
        }
    }
}

}}

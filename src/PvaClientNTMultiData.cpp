/* pvaClientNTMultiData.cpp */
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
#include <pv/pvaClientMultiChannel.h>
#include <pv/standardField.h>
#include <pv/convert.h>
#include <epicsMath.h>

using std::tr1::static_pointer_cast;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::nt;
using namespace std;

namespace epics { namespace pvaClient { 

static ConvertPtr convert = getConvert();
static FieldCreatePtr fieldCreate = getFieldCreate();
static PVDataCreatePtr pvDataCreate = getPVDataCreate();
static StandardFieldPtr standardField = getStandardField();


PvaClientNTMultiDataPtr PvaClientNTMultiData::create(
    epics::pvData::UnionConstPtr const & u,
    PvaClientMultiChannelPtr const &pvaMultiChannel,
    PvaClientChannelArray const &pvaClientChannelArray,
    PVStructurePtr const &  pvRequest)
{
    PvaClientNTMultiDataPtr pvaClientNTMultiData(
         new PvaClientNTMultiData(u,pvaMultiChannel,pvaClientChannelArray,pvRequest));
    return pvaClientNTMultiData;
}

PvaClientNTMultiData::PvaClientNTMultiData(
         epics::pvData::UnionConstPtr const & u,
         PvaClientMultiChannelPtr const &pvaClientMultiChannel,
         PvaClientChannelArray const &pvaClientChannelArray,
         epics::pvData::PVStructurePtr const &  pvRequest)
: pvaClientMultiChannel(pvaClientMultiChannel),
  pvaClientChannelArray(pvaClientChannelArray),
  pvRequest(pvRequest),
  u(u),
  nchannel(pvaClientChannelArray.size()),
  gotAlarm(false),
  gotTimeStamp(false),
  isDestroyed(false)
{
    PVFieldPtr pvValue = pvRequest->getSubField("field.value");
    if(!pvValue) {
        throw std::runtime_error("pvRequest did not specify value");
    }
    topPVStructure.resize(nchannel);
    for(size_t i=0; i< nchannel; ++i) topPVStructure[i] = PVStructurePtr();
    NTMultiChannelBuilderPtr builder = NTMultiChannel::createBuilder();
    builder->value(u);
    if(pvRequest->getSubField("field.alarm"))
    {
         gotAlarm = true;
         builder->addAlarm();
         severity.resize(nchannel);
         status.resize(nchannel);
         message.resize(nchannel);
         
    }
    if(pvRequest->getSubField("field.timeStamp")) {
        gotTimeStamp = true;
        builder->addTimeStamp();
        secondsPastEpoch.resize(nchannel);
        nanoseconds.resize(nchannel);
        userTag.resize(nchannel);
    }
    ntMultiChannel = builder->create();
}


PvaClientNTMultiData::~PvaClientNTMultiData()
{
    destroy();
}

void PvaClientNTMultiData::destroy()
{
    {
        Lock xx(mutex);
        if(isDestroyed) return;
        isDestroyed = true;
    }
    pvaClientChannelArray.clear();
}

void PvaClientNTMultiData::setStructure(StructureConstPtr const & structure,size_t index)
{
    FieldConstPtr field = structure->getField("value");
    if(!field) {
        string message = "channel "
           + pvaClientChannelArray[index]->getChannel()->getChannelName()
           + " does not have top level value field";
        throw std::runtime_error(message);
    }
}

void PvaClientNTMultiData::setPVStructure(
        PVStructurePtr const &pvStructure,size_t index)
{
    topPVStructure[index] = pvStructure;
}


size_t PvaClientNTMultiData::getNumber()
{
    return nchannel;
}

void PvaClientNTMultiData::startDeltaTime()
{
    for(size_t i=0; i<nchannel; ++i)
    {
        topPVStructure[i] = PVStructurePtr();
        if(gotAlarm)
        {
            alarm.setSeverity(noAlarm);
            alarm.setStatus(noStatus);
            alarm.setMessage("");
            severity[i] = invalidAlarm;
            status[i] = undefinedStatus;
            message[i] = "not connected";
        }
        if(gotTimeStamp)
        {
            timeStamp.getCurrent();
            secondsPastEpoch[i] = 0;
            nanoseconds[i] = 0;
            userTag[i] = 0;
        }
    }
}


void PvaClientNTMultiData::endDeltaTime()
{
    for(size_t i=0; i<nchannel; ++i)
    {
        PVStructurePtr pvst = topPVStructure[i];
        if(!pvst) {
              unionValue[i] = PVUnionPtr();
        } else {
              unionValue[i]->set(pvst->getSubField("value"));
        }
        if(gotAlarm)
        {
              severity[i] = pvst->getSubField<PVInt>("alarm.severity")->get();
              status[i] = pvst->getSubField<PVInt>("alarm.status")->get();
              message[i] = pvst->getSubField<PVString>("alarm.message")->get();
        }
        if(gotTimeStamp)
        {
              secondsPastEpoch[i] = pvst->getSubField<PVLong>("timeStamp.secondsPastEpoch")->get();
              nanoseconds[i] = pvst->getSubField<PVInt>("timeStamp.nanoseconds")->get();
              userTag[i] = pvst->getSubField<PVInt>("timeStamp.userTag")->get();
        }
             
    }
    ntMultiChannel->getValue()->replace(freeze(unionValue));
    if(gotAlarm)
    {
        ntMultiChannel->getSeverity()->replace(freeze(severity));
        ntMultiChannel->getStatus()->replace(freeze(status));
        ntMultiChannel->getMessage()->replace(freeze(message));
    }
    if(gotTimeStamp)
    {
        ntMultiChannel->getSecondsPastEpoch()->replace(freeze(secondsPastEpoch));
        ntMultiChannel->getNanoseconds()->replace(freeze(nanoseconds));
        ntMultiChannel->getUserTag()->replace(freeze(userTag));
    }
}

TimeStamp PvaClientNTMultiData::getTimeStamp()
{
    pvTimeStamp.get(timeStamp);
    return timeStamp;
}

NTMultiChannelPtr PvaClientNTMultiData::getNTMultiChannel()
{
    return ntMultiChannel;
}

PVStructurePtr PvaClientNTMultiData::getPVTop()
{
    return ntMultiChannel->getPVStructure();
}

}}

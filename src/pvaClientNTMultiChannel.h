/* pvaClientNTMultiChannel.h */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvData is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
/**
 * @author mrk
 * @date 2015.02
 */
#ifndef PVACLIENTNTMULTIChannel_H
#define PVACLIENTNTMULTIChannel_H

#ifdef epicsExportSharedSymbols
#   define pvaClientEpicsExportSharedSymbols
#   undef epicsExportSharedSymbols
#endif

#include <pv/pvaClient.h>

namespace epics { namespace pvaClient { 

class PvaClientNTMultiChannel;
typedef std::tr1::shared_ptr<PvaClientNTMultiChannel> PvaClientNTMultiChannelPtr;

/**
 * @brief Support for multiple channels where each channel has a value field that
 * is a scalar, scalarArray, or enumerated structure.
 * The data is provided via normativeType NTMultiChannel.
 * If any problems arise an exception is thrown.
 *
 * @author mrk
 */
class epicsShareClass PvaClientNTMultiChannel 
{
public:
    POINTER_DEFINITIONS(PvaClientNTMultiChannel);
    /**
     * @brief Create a PvaClientNTMultiChannel.
     * @param &pvaClient Interface to PvaClient
     * @param channelName PVStringArray of channelNames.
     * @param structure valid NTMultiChannel structure.
     * @param timeout Timeout for connecting.
     * @param providerName The provider for each channel.
     * @return The interface to PvaClientNTMultiChannel.
     */
    static PvaClientNTMultiChannelPtr create(
        PvaClientPtr const & pvaClient,
        epics::pvData::PVStringArrayPtr const & channelName,
        epics::pvData::StructureConstPtr const & structure,
        double timeout = 5.0,
        std::string const & providerName = "pva");
    /**
     * @brief destructor
     */
    ~PvaClientNTMultiChannel();
    /** 
     * @brief destroy any resources used.
     */
    void destroy();
    /** 
     * @brief get the value of all the channels.
     * @return The data.
     */
    epics::nt::NTMultiChannelPtr get();
    /** 
     * @brief put a new value to each  channel.
     * @param value The data.
     */
    void put(epics::nt::NTMultiChannelPtr const &value);
    /** 
     * @brief Get the PvaClientMultiChannel.
     * @return The interface.
     */
    PvaClientMultiChannelPtr getPvaClientMultiChannel();
private:
    PvaClientNTMultiChannel(
        PvaClientMultiChannelPtr const & channelName,
        epics::nt::NTMultiChannelPtr const &ntMultiChannel);
    void createGet();
    void createPut();

    PvaClientMultiChannelPtr pvaClientMultiChannel;
    epics::nt::NTMultiChannelPtr ntMultiChannel;
    epics::pvData::PVUnionArrayPtr pvUnionArray;
    epics::pvData::PVDataCreatePtr pvDataCreate;
    std::vector<PvaClientGetPtr> pvaClientGet;
    std::vector<PvaClientPutPtr> pvaClientPut;
    epics::pvData::shared_vector<epics::pvData::int32> severity;
    epics::pvData::shared_vector<epics::pvData::int32> status;
    epics::pvData::shared_vector<std::string> message;
    epics::pvData::shared_vector<epics::pvData::int64> secondsPastEpoch;
    epics::pvData::shared_vector<epics::pvData::int32> nanoseconds;
    epics::pvData::shared_vector<epics::pvData::int32> userTag;
    epics::pvData::Alarm alarm;
    epics::pvData::PVAlarm pvAlarm;
    epics::pvData::TimeStamp timeStamp;;
    epics::pvData::PVTimeStamp pvTimeStamp;
};

}}

#endif // PVACLIENTNTMULTIChannel_H

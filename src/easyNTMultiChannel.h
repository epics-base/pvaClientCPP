/* easyNTMultiChannel.h */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvData is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
/**
 * @author mrk
 * @date 2015.02
 */
#ifndef EASYNTMULTIChannel_H
#define EASYNTMULTIChannel_H

#ifdef epicsExportSharedSymbols
#   define easyPVAEpicsExportSharedSymbols
#   undef epicsExportSharedSymbols
#endif

#include <pv/easyPVA.h>

namespace epics { namespace easyPVA { 

class EasyNTMultiChannel;
typedef std::tr1::shared_ptr<EasyNTMultiChannel> EasyNTMultiChannelPtr;

/**
 * @brief Support for multiple channels where each channel has a value field that
 * is a scalar, scalarArray, or enumerated structure.
 * The data is provided via normativeType NTMultiChannel.
 * If any problems arise an exception is thrown.
 *
 * @author mrk
 */
class epicsShareClass EasyNTMultiChannel 
{
public:
    POINTER_DEFINITIONS(EasyNTMultiChannel);
    /**
     * @brief Create a EasyNTMultiChannel.
     * @param &easyPVA Interface to EasyPVA
     * @param channelName PVStringArray of channelNames.
     * @param structure valid NTMultiChannel structure.
     * @param timeout Timeout for connecting.
     * @param providerName The provider for each channel.
     * @return The interface to EasyNTMultiChannel.
     */
    static EasyNTMultiChannelPtr create(
        EasyPVAPtr const & easyPVA,
        epics::pvData::PVStringArrayPtr const & channelName,
        epics::pvData::StructureConstPtr const & structure,
        double timeout = 5.0,
        std::string const & providerName = "pva");
    /**
     * @brief destructor
     */
    ~EasyNTMultiChannel();
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
     * @brief Get the EasyMultiChannel.
     * @return The interface.
     */
    EasyMultiChannelPtr getEasyMultiChannel();
private:
    EasyNTMultiChannel(
        EasyMultiChannelPtr const & channelName,
        epics::nt::NTMultiChannelPtr const &ntMultiChannel);
    void createGet();
    void createPut();

    EasyMultiChannelPtr easyMultiChannel;
    epics::nt::NTMultiChannelPtr ntMultiChannel;
    epics::pvData::PVUnionArrayPtr pvUnionArray;
    epics::pvData::PVDataCreatePtr pvDataCreate;
    std::vector<EasyGetPtr> easyGet;
    std::vector<EasyPutPtr> easyPut;
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

#endif // EASYNTMULTIChannel_H

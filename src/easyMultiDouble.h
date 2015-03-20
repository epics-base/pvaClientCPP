/* easyMultiDouble.h */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvData is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
/**
 * @author mrk
 * @date 2015.02
 */
#ifndef EASYMULTIDOUBLE_H
#define EASYMULTIDOUBLE_H

#ifdef epicsExportSharedSymbols
#   define easyPVAEpicsExportSharedSymbols
#   undef epicsExportSharedSymbols
#endif

#include <pv/easyPVA.h>

namespace epics { namespace easyPVA { 

class EasyMultiDouble;
typedef std::tr1::shared_ptr<EasyMultiDouble> EasyMultiDoublePtr;

/**
 * @brief Support for multiple channels where each channel has a value field that is a scalar double.
 * If any problems arise an exception is thrown.
 *
 * @author mrk
 */
class epicsShareClass EasyMultiDouble 
{
public:
    POINTER_DEFINITIONS(EasyMultiDouble);
    /**
     * @brief Create a EasyMultiDouble.
     * @param &easyPVA Interface to EasyPVA
     * @param channelName PVStringArray of channelNames.
     * @param timeout The timeout in seconds for connecting.
     * @param providerName The name of the channelProvider for each channel.
     * @return The interface to EasyMultiDouble.
     */
    static EasyMultiDoublePtr create(
        EasyPVAPtr const & easyPVA,
        epics::pvData::PVStringArrayPtr const & channelName,
        double timeout = 5.0,
        std::string const & providerName = "pva");
    /**
     * @brief destructor
     */
    ~EasyMultiDouble();
    /** 
     * @brief destroy any resources used.
     */
    void destroy();
    /** 
     * @brief get the value of all the channels.
     * @return The data.
     */
    epics::pvData::shared_vector<double> get();
    /** 
     * @brief put a new value to each  channel.
     * @param value The data.
     */
    void put(epics::pvData::shared_vector<double> const &value);
    EasyMultiChannelPtr getEasyMultiChannel();
private:
    EasyMultiDouble(
        EasyMultiChannelPtr const & channelName);
    void createGet();
    void createPut();

    EasyMultiChannelPtr easyMultiChannel;
    std::vector<EasyGetPtr> easyGet;
    std::vector<EasyPutPtr> easyPut;
};

}}

#endif // EASYMULTIDOUBLE_H

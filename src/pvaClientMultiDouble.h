/* pvaClientMultiDouble.h */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvData is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
/**
 * @author mrk
 * @date 2015.02
 */
#ifndef PVACLIENTMULTIDOUBLE_H
#define PVACLIENTMULTIDOUBLE_H

#ifdef epicsExportSharedSymbols
#   define pvaClientEpicsExportSharedSymbols
#   undef epicsExportSharedSymbols
#endif

#include <pv/pvaClient.h>

namespace epics { namespace pvaClient { 

class PvaClientMultiDouble;
typedef std::tr1::shared_ptr<PvaClientMultiDouble> PvaClientMultiDoublePtr;

/** Support for multiple channels where each channel has a value field that is a scalar double.
 * If any problems arise an exception is thrown.
 *
 * @author mrk
 */
class epicsShareClass PvaClientMultiDouble 
{
public:
    POINTER_DEFINITIONS(PvaClientMultiDouble);
    /** Create a PvaClientMultiDouble.
     * @param &pvaClient Interface to PvaClient
     * @param channelName PVStringArray of channelNames.
     * @param timeout The timeout in seconds for connecting.
     * @param providerName The name of the channelProvider for each channel.
     * @return The interface to PvaClientMultiDouble.
     */
    static PvaClientMultiDoublePtr create(
        PvaClientPtr const & pvaClient,
        epics::pvData::PVStringArrayPtr const & channelName,
        double timeout = 5.0,
        std::string const & providerName = "pva");
    /** Destructor
     */
    ~PvaClientMultiDouble();
    /** Destroy all resources used.
     */
    void destroy();
    /** Get the value of all the channels.
     * @return The data.
     */
    epics::pvData::shared_vector<double> get();
    /** Put a new value to each  channel.
     * @param value The data.
     */
    void put(epics::pvData::shared_vector<double> const &value);
    PvaClientMultiChannelPtr getPvaClientMultiChannel();
private:
    PvaClientMultiDouble(
        PvaClientMultiChannelPtr const & channelName);
    void createGet();
    void createPut();

    PvaClientMultiChannelPtr pvaClientMultiChannel;
    std::vector<PvaClientGetPtr> pvaClientGet;
    std::vector<PvaClientPutPtr> pvaClientPut;
};

}}

#endif // PVACLIENTMULTIDOUBLE_H

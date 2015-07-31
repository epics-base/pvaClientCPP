/* pvaClient.h */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvData is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
/**
 * @author mrk
 * @date 2015.02
 */
#ifndef PVACLIENTMULTICHANNEL_H
#define PVACLIENTMULTICHANNEL_H

#ifdef epicsExportSharedSymbols
#   define pvaClientEpicsExportSharedSymbols
#   undef epicsExportSharedSymbols
#endif

#include <pv/pvaClient.h>
#include <pv/ntmultiChannel.h>


namespace epics { namespace pvaClient { 


class PvaClientMultiChannel;
typedef std::tr1::shared_ptr<PvaClientMultiChannel> PvaClientMultiChannelPtr;
class PvaClientMultiGetDouble;
typedef std::tr1::shared_ptr<PvaClientMultiGetDouble> PvaClientMultiGetDoublePtr;
class PvaClientMultiPutDouble;
typedef std::tr1::shared_ptr<PvaClientMultiPutDouble> PvaClientMultiPutDoublePtr;
class PvaClientMultiMonitorDouble;
typedef std::tr1::shared_ptr<PvaClientMultiMonitorDouble> PvaClientMultiMonitorDoublePtr;


typedef epics::pvData::shared_vector<PvaClientChannelPtr> PvaClientChannelArray;

/**
 * Provides access to multiple channels.
 *
 * @author mrk
 */
class epicsShareClass PvaClientMultiChannel :
    public std::tr1::enable_shared_from_this<PvaClientMultiChannel>
{
public:
    POINTER_DEFINITIONS(PvaClientMultiChannel);
    /** Create a PvaClientMultiChannel.
     * @param pvaClient The interface to pvaClient.
     * @param channelNames The names of the channel..
     * @param providerName The name of the provider.
     * @param maxNotConnected The maximum number of channels that can be disconnected.
     * @return The interface to the PvaClientMultiChannel
     */
    static PvaClientMultiChannelPtr create(
         PvaClientPtr const &pvaClient,
         epics::pvData::shared_vector<const std::string> const & channelNames,
         std::string const & providerName = "pva",
         size_t maxNotConnected=0
     );

    ~PvaClientMultiChannel();
    /** Destroy the pvAccess connection.
     */
    void destroy();
    /** Get the channelNames.
     * @return The names.
     */
    epics::pvData::shared_vector<const std::string> getChannelNames();
    /** Connect to the channels.
     * This calls issueConnect and waitConnect.
     * An exception is thrown if connect fails.
     * @param timeout The time to wait for connecting to the channel.
     * @return status of request
     */
    epics::pvData::Status connect(double timeout=5);
    /** Are all channels connected?
     * @return if all are connected.
     */
    bool allConnected();
    /** Has a connection state change occured?
     * @return (true, false) if (at least one, no) channel has changed state.
     */
    bool connectionChange();
    /** Get the connection state of each channel.
     * @return The state of each channel.
     */
    epics::pvData::shared_vector<bool> getIsConnected();
    /** Get the pvaClientChannelArray.
     * @return The shared pointer.
     */
    PvaClientChannelArray getPvaClientChannelArray();
    /** Get pvaClient.
     * @return The shared pointer.
     */
    PvaClientPtr getPvaClient();
    /**
     * create a pvaClientMultiGetDouble
     * @return The interface.
     */
    PvaClientMultiGetDoublePtr createGet();   
    /**
     * create a pvaClientMultiPutDouble
     * @return The interface.
     */
    PvaClientMultiPutDoublePtr createPut();
    /**
     * Create a pvaClientMultiMonitorDouble.
     * @return The interface.
     */
    PvaClientMultiMonitorDoublePtr createMonitor();
    /** Get the shared pointer to self.
     * @return The shared pointer.
     */
    PvaClientMultiChannelPtr getPtrSelf()
    {
        return shared_from_this();
    }
private:
    PvaClientMultiChannel(
        PvaClientPtr const &pvaClient,
        epics::pvData::shared_vector<const std::string> const & channelName,
        std::string const & providerName,
        size_t maxNotConnected);

    void checkConnected();
    
    PvaClientPtr pvaClient;
    epics::pvData::shared_vector<const std::string> channelName;
    std::string providerName;
    size_t maxNotConnected;

    size_t numChannel;
    epics::pvData::Mutex mutex;

    size_t numConnected;
    PvaClientChannelArray pvaClientChannelArray;
    epics::pvData::shared_vector<bool> isConnected;
    bool isDestroyed;
};

/**
 *  This provides channelGet to multiple channels where each channel has a numeric scalar value field.
 */
class epicsShareClass PvaClientMultiGetDouble :
    public std::tr1::enable_shared_from_this<PvaClientMultiGetDouble>
{

public:
    POINTER_DEFINITIONS(PvaClientMultiGetDouble);
    
    /**
     * Factory method that creates a PvaClientMultiGetDouble.
     * @param pvaClientMultiChannel The interface to PvaClientMultiChannel.
     * @param pvaClientChannelArray The PvaClientChannel array.
     * @return The interface.
     */
    static PvaClientMultiGetDoublePtr create(
         PvaClientMultiChannelPtr const &pvaClientMultiChannel,
         PvaClientChannelArray const &pvaClientChannelArray);

    ~PvaClientMultiGetDouble();

    /** Destroy the pvAccess connection.
     */
    void destroy();
     /**
     * Create a channelGet for each channel.
     */
    void connect();
    /**
     * get the data.
     * @return The double[] where each element is the value field of the corresponding channel.
     */
    epics::pvData::shared_vector<double> get();
    /** Get the shared pointer to self.
     * @return The shared pointer.
     */
    PvaClientMultiGetDoublePtr getPtrSelf()
    {
        return shared_from_this();
    }
private:
    PvaClientMultiGetDouble(
         PvaClientMultiChannelPtr const &pvaClientMultiChannel,
         PvaClientChannelArray const &pvaClientChannelArray);

    PvaClientMultiChannelPtr pvaClientMultiChannel;
    PvaClientChannelArray pvaClientChannelArray;
    size_t nchannel;
    epics::pvData::Mutex mutex;
    
    epics::pvData::shared_vector<double> doubleValue;
    std::vector<PvaClientGetPtr> pvaClientGet;
    bool isGetConnected;
    bool isDestroyed;
};

/**
 * This provides channelPut to multiple channels where each channel has a numeric scalar value field.
 */
class epicsShareClass PvaClientMultiPutDouble :
    public std::tr1::enable_shared_from_this<PvaClientMultiPutDouble>
{

public:
    POINTER_DEFINITIONS(PvaClientMultiPutDouble);
   
    /**
     * Factory method that creates a PvaClientMultiPutDouble.
     * @param pvaClientMultiChannel The interface to PvaClientMultiChannel.
     * @param pvaClientChannelArray The PvaClientChannel array.
     * @return The interface.
     */
    static PvaClientMultiPutDoublePtr create(
         PvaClientMultiChannelPtr const &pvaMultiChannel,
         PvaClientChannelArray const &pvaClientChannelArray);
    ~PvaClientMultiPutDouble();

    /** Destroy the pvAccess connection.
     */
    void destroy();
     /**
     * Create a channelPut for each channel.
     */
    void connect();
    /** put data to each channel as a double
     * @param data The array of data for each channel.
     */
    void put(epics::pvData::shared_vector<double> const &data);
    /** Get the shared pointer to self.
     * @return The shared pointer.
     */
    PvaClientMultiPutDoublePtr getPtrSelf()
    {
        return shared_from_this();
    }
private:
    PvaClientMultiPutDouble(
         PvaClientMultiChannelPtr const &pvaClientMultiChannel,
         PvaClientChannelArray const &pvaClientChannelArray);

    PvaClientMultiChannelPtr pvaClientMultiChannel;
    PvaClientChannelArray pvaClientChannelArray;
    size_t nchannel;
    epics::pvData::Mutex mutex;

    std::vector<PvaClientPutPtr> pvaClientPut;
    bool isPutConnected;
    bool isDestroyed;
};

/**
 * This provides a monitor to multiple channels where each channel has a numeric scalar value field.
 */
class epicsShareClass PvaClientMultiMonitorDouble :
    public std::tr1::enable_shared_from_this<PvaClientMultiMonitorDouble>
{

public:
    POINTER_DEFINITIONS(PvaClientMultiMonitorDouble);
    
    /**
     * Factory method that creates a PvaClientMultiMonitorDouble.
     * @param pvaClientMultiChannel The interface to PvaClientMultiChannel.
     * @param pvaClientChannelArray The PvaClientChannel array.
     * @return The interface.
     */
    static PvaClientMultiMonitorDoublePtr create(
         PvaClientMultiChannelPtr const &pvaMultiChannel,
         PvaClientChannelArray const &pvaClientChannelArray);
    ~PvaClientMultiMonitorDouble();

    /** Destroy the pvAccess connection.
     */
    void destroy();
     /**
     * Create a channel monitor for each channel.
     */
    void connect();
     /**
     * poll each channel.
     * If any has new data it is used to update the double[].
     * @return (false,true) if (no, at least one) value was updated.
     */
    bool poll();
    /**
     * Wait until poll returns true.
     * @param waitForEvent The time to keep trying.
     * A thread sleep of .1 seconds occurs between each call to poll.
     * @return (false,true) if (timeOut, poll returned true).
     */
    bool waitEvent(double waitForEvent);
    /**
     * get the data.
     *  @return The double[] where each element is the value field of the corresponding channel.
     */
    epics::pvData::shared_vector<double> get();
    /** Monitor the shared pointer to self.
     * @return The shared pointer.
     */
    PvaClientMultiMonitorDoublePtr getPtrSelf()
    {
        return shared_from_this();
    }
private:
    PvaClientMultiMonitorDouble(
         PvaClientMultiChannelPtr const &pvaClientMultiChannel,
         PvaClientChannelArray const &pvaClientChannelArray);

    PvaClientMultiChannelPtr pvaClientMultiChannel;
    PvaClientChannelArray pvaClientChannelArray;
    size_t nchannel;
    epics::pvData::Mutex mutex;

    epics::pvData::shared_vector<double> doubleValue;
    std::vector<PvaClientMonitorPtr> pvaClientMonitor;
    bool isMonitorConnected;
    bool isDestroyed;
};

}}

#endif  /* PVACLIENTMULTICHANNEL_H */

/** @page Overview Documentation
 *
 * <a href = "../pvaClientOverview.html">pvaClientOverview.html</a>
 *
 */


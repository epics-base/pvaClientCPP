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
#ifndef PVACLIENT_H
#define PVACLIENT_H

#ifdef epicsExportSharedSymbols
#   define pvaClientEpicsExportSharedSymbols
#   undef epicsExportSharedSymbols
#endif

#include <list>
#include <iostream>
#include <pv/requester.h>
#include <pv/status.h>
#include <pv/event.h>
#include <pv/lock.h>
#include <pv/pvData.h>
#include <pv/pvCopy.h>
#include <pv/pvTimeStamp.h>
#include <pv/timeStamp.h>
#include <pv/pvAlarm.h>
#include <pv/alarm.h>
#include <pv/pvAccess.h>
#include <pv/standardField.h>
#include <pv/standardPVField.h>
#include <pv/createRequest.h>
#include <pv/nt.h>

#ifdef pvaClientEpicsExportSharedSymbols
#   define epicsExportSharedSymbols
#	undef pvaClientEpicsExportSharedSymbols
#endif

#include <shareLib.h>


namespace epics { namespace pvaClient { 

class PvaClient;
typedef std::tr1::shared_ptr<PvaClient> PvaClientPtr;
class PvaClientGetData;
typedef std::tr1::shared_ptr<PvaClientGetData> PvaClientGetDataPtr;
class PvaClientPutData;
typedef std::tr1::shared_ptr<PvaClientPutData> PvaClientPutDataPtr;
class PvaClientMonitorData;
typedef std::tr1::shared_ptr<PvaClientMonitorData> PvaClientMonitorDataPtr;
class PvaClientChannel;
typedef std::tr1::shared_ptr<PvaClientChannel> PvaClientChannelPtr;
class PvaClientField;
typedef std::tr1::shared_ptr<PvaClientField> PvaClientFieldPtr;
class PvaClientProcess;
typedef std::tr1::shared_ptr<PvaClientProcess> PvaClientProcessPtr;
class PvaClientGet;
typedef std::tr1::shared_ptr<PvaClientGet> PvaClientGetPtr;
class PvaClientPut;
typedef std::tr1::shared_ptr<PvaClientPut> PvaClientPutPtr;
class PvaClientPutGet;
typedef std::tr1::shared_ptr<PvaClientPutGet> PvaClientPutGetPtr;
class PvaClientMonitor;
typedef std::tr1::shared_ptr<PvaClientMonitor> PvaClientMonitorPtr;
class PvaClientMonitorRequester;
typedef std::tr1::shared_ptr<PvaClientMonitorRequester> PvaClientMonitorRequesterPtr;
class PvaClientArray;
typedef std::tr1::shared_ptr<PvaClientArray> PvaClientArrayPtr;
class PvaClientRPC;
typedef std::tr1::shared_ptr<PvaClientRPC> PvaClientRPCPtr;

typedef epics::pvData::shared_vector<const PvaClientChannelPtr> PvaClientChannelArray;
typedef std::tr1::shared_ptr<PvaClientChannelArray> PvaClientChannelArrayPtr;
typedef std::tr1::weak_ptr<PvaClientChannelArray> PvaClientChannelArrayWPtr;

class PvaClientMultiChannel;
typedef std::tr1::shared_ptr<PvaClientMultiChannel> PvaClientMultiChannelPtr;
class PvaClientMultiChannelGet;

// following are private to pvaClient
class PvaClientChannelCache;
typedef std::tr1::shared_ptr<PvaClientChannelCache> PvaClientChannelCachePtr;

/**
 * @brief PvaClient is a synchronous interface to pvAccess plus convenience methods.
 *
 * @author mrk
 */
class epicsShareClass PvaClient :
     public epics::pvData::Requester,
     public std::tr1::enable_shared_from_this<PvaClient>
{
public:
    POINTER_DEFINITIONS(PvaClient);

    /**
     * Destructor
     */
    ~PvaClient();
    /**
     * @brief Create an instance of PvaClient
     * @return shared_ptr to new instance.
     */
    static PvaClientPtr create();
    /** @brief get the requester name.
     * @return The name.
     */
    std::string getRequesterName();
    /**
     * @brief A new message.
     * If a requester is set then it is called otherwise message is displayed
     * on standard out.
     * @param message The message.
     * @param messageType The type.
     */
    void message(
        std::string const & message,
        epics::pvData::MessageType messageType);
    /**
     * @brief Destroy all the channels and multiChannels.
     */
    void destroy();
    /**
     * @brief get a cached channel or create and connect to a new channel.
     * The provider is pvaClient. The timeout is 5 seconds.
     * If connection can not be made an exception is thrown.
     * @param channelName The channelName.
     * @return The interface.
     */
    PvaClientChannelPtr channel(std::string const & channelName)
    { return channel(channelName,"pva", 5.0); }
    /**
     * @brief get a cached channel or create and connect to a new channel.
     * If connection can not be made an exception is thrown.
     * @param channelName The channelName.
     * @return The interface.
     */
    PvaClientChannelPtr channel(
        std::string const & channelName,
        std::string const &providerName,
        double timeOut);
    /**
     * @brief Create an PvaClientChannel. The provider is pvaClient.
     * @param channelName The channelName.
     * @return The interface.
     */
    PvaClientChannelPtr createChannel(std::string const & channelName);
    /**
     * @brief Create an PvaClientChannel with the specified provider.
     * @param channelName The channelName.
     * @param providerName The provider.
     * @return The interface or null if the provider does not exist.
     */
    PvaClientChannelPtr createChannel(
       std::string const & channelName,
       std::string const & providerName);
    /**
     * @brief Create an PvaClientMultiChannel. The provider is pvAccess.
     * @param channelName The channelName array.
     * @return The interface.
     */
    PvaClientMultiChannelPtr createMultiChannel(
        epics::pvData::PVStringArrayPtr const & channelNames);
    /**
     * @brief Create an PvaClientMultiChannel with the specified provider.
     * @param channelName The channelName array.
     * @param providerName The provider.
     * @return The interface.
     */
    PvaClientMultiChannelPtr createMultiChannel(
        epics::pvData::PVStringArrayPtr const & channelNames,
        std::string const & providerName);
    /**
     * @brief Set a requester.
     * The default is for PvaClient to handle messages by printing to System.out.
     * @param requester The requester.
     */
    void setRequester(epics::pvData::RequesterPtr const & requester);
    /**
     * @brief Clear the requester. PvaClient will handle messages.
     */
    void clearRequester();
    /**
     * @brief get shared pointer to this
     */
    PvaClientPtr getPtrSelf()
    {
        return shared_from_this();
    }
private:
    PvaClient();
    PvaClientChannelCachePtr pvaClientChannelCache;

    epics::pvData::PVStructurePtr createRequest(std::string const &request);
    std::list<PvaClientChannelPtr> channelList;
    std::list<PvaClientMultiChannelPtr> multiChannelList;
    epics::pvData::Requester::weak_pointer requester;
    bool isDestroyed;
    epics::pvData::Mutex mutex;
};

// folowing private to PvaClientChannel
class PvaClientGetCache;
typedef std::tr1::shared_ptr<PvaClientGetCache> PvaClientGetCachePtr;
class PvaClientPutCache;
typedef std::tr1::shared_ptr<PvaClientPutCache> PvaClientPutCachePtr;
class ChannelRequesterImpl;
/**
 * @brief An easy to use alternative to directly calling the Channel methods of pvAccess.
 *
 * @author mrk
 */
class epicsShareClass PvaClientChannel :
    public std::tr1::enable_shared_from_this<PvaClientChannel>
{
public:
    POINTER_DEFINITIONS(PvaClientChannel);
    /**
     * @brief Create a PvaClientChannel.
     * @param pvaClient Interface to PvaClient
     * @param channelName The name of the channel.
     * @return The interface.
     */
    static PvaClientChannelPtr create(
        PvaClientPtr const &pvaClient,
        std::string const & channelName)
        {return create(pvaClient,channelName,"pva");}
    /**
     * @brief Create a PvaClientChannel.
     * @param channelName The name of the channel.
     * @param providerName The name of the provider.
     * @return The interface to the PvaClientStructure.
     */
    static PvaClientChannelPtr create(
         PvaClientPtr const &pvaClient,
         std::string const & channelName,
         std::string const & providerName);
    ~PvaClientChannel();
    /**
     * @brief Destroy the pvAccess connection.
     */
    void destroy();
    /**
     * @brief Get the name of the channel to which PvaClientChannel is connected.
     * @return The channel name.
     */
    std::string getChannelName();
    /**
     * @brief Get the the channel to which PvaClientChannel is connected.
     * @return The channel interface.
     */
    epics::pvAccess::Channel::shared_pointer getChannel();
    /**
     * @brief Connect to the channel.
     * This calls issueConnect and waitConnect.
     * An exception is thrown if connect fails.
     * @param timeout The time to wait for connecting to the channel.
     */
    void connect(double timeout=5.0);
    /**
     * @brief Issue a connect request and return immediately.
     */
    void issueConnect();
    /**
     * @brief Wait until the connection completes or for timeout.
     * @param timeout The time in second to wait.
     * @return status.
     */
    epics::pvData::Status waitConnect(double timeout);
    /**
     * @brief Calls the next method with subField = "";
     * @return The interface.
     */
    PvaClientFieldPtr createField();
    /**
     * @brief Create an PvaClientField for the specified subField.
     * @param subField The syntax for subField is defined in package org.epics.pvdata.copy
     * @return The interface.
     */
    PvaClientFieldPtr createField(std::string const & subField);
    /**
     * @brief Calls the next method with request = "";
     * @return The interface.
     */
    PvaClientProcessPtr createProcess();
    /**
     * @brief First call createRequest as implemented by pvDataJava and then calls the next method.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    PvaClientProcessPtr createProcess(std::string const & request);
    /**
     * @brief Creates an PvaClientProcess. 
     * @param pvRequest The syntax of pvRequest is described in package org.epics.pvdata.copy.
     * @return The interface.
     */
    PvaClientProcessPtr createProcess(epics::pvData::PVStructurePtr const &  pvRequest);
    /**
     * @brief Call the next method with request =  "field(value,alarm,timeStamp)" 
     * @return The interface.
     */
    PvaClientGetPtr get();
    /**
     * @brief get a cached PvaClientGet or create and connect to a new PvaClientGet.
     * Then call it's get method.
     * If connection can not be made an exception is thrown.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    PvaClientGetPtr get(std::string const & request);
    /**
     * @brief Call the next method with request =  "field(value,alarm,timeStamp)" 
     * @return The interface.
     */
    PvaClientGetPtr createGet();
    /**
     * @brief First call createRequest as implemented by pvDataJava and then call the next method.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    PvaClientGetPtr createGet(std::string const & request);
    /**
     * @brief Creates an PvaClientGet.
     * @param pvRequest The syntax of pvRequest is described in package org.epics.pvdata.copy.
     * @return The interface.
     */
    PvaClientGetPtr createGet(epics::pvData::PVStructurePtr const &  pvRequest);
    /**
     * @brief Call the next method with request =  "field(value)" 
     * @return The interface.
     */
    PvaClientPutPtr put();
    /**
     *  @brief get a cached PvaClientPut or create and connect to a new PvaClientPut.
     *  Then call it's get method.
     *  If connection can not be made an exception is thrown.
     *  @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    PvaClientPutPtr put(std::string const & request);
    /**
     *  @brief Call the next method with request = "field(value)" 
     * @return The interface.
     */
    PvaClientPutPtr createPut();
    /**
     * @brief First call createRequest as implemented by pvDataJava and then calls the next method.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    PvaClientPutPtr createPut(std::string const & request);
    /**
     * @brief Create an PvaClientPut.
     * @param pvRequest The syntax of pvRequest is described in package org.epics.pvdata.copy.
     * @return The interface.
     */
    PvaClientPutPtr createPut(epics::pvData::PVStructurePtr const & pvRequest);
    /**
     *  @brief Call the next method with request = "record[process=true]putField(argument)getField(result)".
     * @return The interface.
     */
    PvaClientPutGetPtr createPutGet();
    /**
     * @brief First call createRequest as implemented by pvDataJava and then calls the next method.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    PvaClientPutGetPtr createPutGet(std::string const & request);
    /**
     * @brief Create an PvaClientPutGet.
     * @param pvRequest The syntax of pvRequest is described in package org.epics.pvdata.copy.
     * @return The interface.
     */
    PvaClientPutGetPtr createPutGet(epics::pvData::PVStructurePtr const & pvRequest);
    /**
     * @brief Call createRPC(PVStructure(null))
     * @return The interface.
     */
    PvaClientRPCPtr createRPC();
    /**
     * @brief First call createRequest as implemented by pvDataJava and then calls the next method.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    PvaClientRPCPtr createRPC(std::string const & request);
    /**
     * @brief Create an PvaClientRPC.
     * @param pvRequest The syntax of pvRequest is described in package org.epics.pvdata.copy.
     * @return The interface.
     */
    PvaClientRPCPtr createRPC(epics::pvData::PVStructurePtr const & pvRequest);
    /**
     * @brief Call the next method with request = "field(value)";
     * @return The interface.
     */
    PvaClientArrayPtr createArray();
    /**
     * @brief First call createRequest as implemented by pvDataJava and then calls the next method.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    PvaClientArrayPtr createArray(std::string const & request);
    /**
     * @brief Create an PvaClientArray.
     * @param pvRequest The syntax of pvRequest is described in package org.epics.pvdata.copy.
     * @return The interface.
     */
    PvaClientArrayPtr createArray(epics::pvData::PVStructurePtr const &  pvRequest);
    /**
     * @brief Call the next method with request =  "field(value,alarm,timeStamp)" 
     * @return The interface.
     */
    PvaClientMonitorPtr monitor();
    /**
     * @brief get a cached PvaClientMonitor or create and connect to a new PvaClientMonitor.
     * Then call it's start method.
     * If connection can not be made an exception is thrown.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    PvaClientMonitorPtr monitor(std::string const & request);
    /**
      * @brief Call the next method with request =  "field(value,alarm,timeStamp)" 
      * @param pvaClientMonitorRequester The client callback.
      * @return The interface.
      */
    PvaClientMonitorPtr monitor(PvaClientMonitorRequesterPtr const & pvaClientMonitorRequester);

    /**
     * @brief get a cached PvaClientMonitor or create and connect to a new PvaClientMonitor.
     * Then call it's start method.
     * If connection can not be made an exception is thrown.
     * @param request The request as described in package org.epics.pvdata.copy
     * @param pvaClientMonitorRequester The client callback.
     * @return The interface.
     */
    PvaClientMonitorPtr monitor(
        std::string const & request,
        PvaClientMonitorRequesterPtr const & pvaClientMonitorRequester);
    /**
     * @brief Call the next method with request = "field(value.alarm,timeStamp)" 
     * @return The interface.
     */
    PvaClientMonitorPtr createMonitor();
    /**
     * @brief First call createRequest as implemented by pvDataJava and then calls the next method.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    PvaClientMonitorPtr createMonitor(std::string const & request);
    /**
     * @brief Create an PvaClientMonitor.
     * @param pvRequest  The syntax of pvRequest is described in package org.epics.pvdata.copy.
     * @return The interface.
     */
    PvaClientMonitorPtr createMonitor(epics::pvData::PVStructurePtr const &  pvRequest);
    PvaClientChannelPtr getPtrSelf()
    {
        return shared_from_this();
    }
private:
    PvaClientChannel(
        PvaClientPtr const &pvaClient,
        std::string const & channelName,
        std::string const & providerName);
    void channelCreated(
        const epics::pvData::Status& status,
        epics::pvAccess::Channel::shared_pointer const & channel);
    void channelStateChange(
        epics::pvAccess::Channel::shared_pointer const & channel,
        epics::pvAccess::Channel::ConnectionState connectionState);
    std::string getRequesterName();
    void message(
        std::string const & message,
        epics::pvData::MessageType messageType);

    enum ConnectState {connectIdle,connectActive,notConnected,connected};

    PvaClient::weak_pointer pvaClient;
    std::string channelName;
    std::string providerName;
    ConnectState connectState;
    bool isDestroyed;
    epics::pvData::CreateRequest::shared_pointer createRequest;
    PvaClientGetCachePtr pvaClientGetCache;
    PvaClientPutCachePtr pvaClientPutCache;

    epics::pvData::Status channelConnectStatus;
    epics::pvData::Mutex mutex;
    epics::pvData::Event waitForConnect;
    epics::pvAccess::Channel::shared_pointer channel;
    epics::pvAccess::ChannelRequester::shared_pointer channelRequester;
    friend class ChannelRequesterImpl;
};

/**
 * @brief This is a class that holds data returned by PvaClientGet or PvaClientPutGet
 *
 */
class epicsShareClass PvaClientGetData
{
public:
    POINTER_DEFINITIONS(PvaClientGetData);
    /**
     * @brief Factory method for creating an instance of PvaClientGetData.
     */
    static PvaClientGetDataPtr create(epics::pvData::StructureConstPtr const & structure);
    ~PvaClientGetData() {}
    /**
     * @brief Set a prefix for throw messages.
     * @param value The prefix.
     */
    void setMessagePrefix(std::string const & value);
   /** @brief Get the structure.
    * @return the structure.
    */
   epics::pvData::StructureConstPtr getStructure();
   /** @brief Get the pvStructure.
    * @return the pvStructure.
    */
   epics::pvData::PVStructurePtr getPVStructure();
   /** @brief Get the BitSet for the pvStructure
    * This shows which fields have changed value.
    * @return The bitSet
    */
   epics::pvData::BitSetPtr getBitSet();
   /** @brief show the fields that have changed.
    * @param out The stream that shows the changed fields.
    * @return The stream that was input
    */
   std::ostream & showChanged(std::ostream & out);
    /**
     * @brief New data is present.
     * @param pvStructureFrom The new data.
     * @param bitSetFrom the bitSet showing which values have changed.
     */
    void setData(
        epics::pvData::PVStructurePtr const & pvStructureFrom,
        epics::pvData::BitSetPtr const & bitSetFrom);
    /**
     * @brief Is there a top level field named value.
     * @return The answer.
     */
    bool hasValue();
    /**
     * @brief Is the value field a scalar?
     * @return The answer.
     */
    bool isValueScalar();
    /**
     * @brief Is the value field a scalar array?
     * @return The answer.
     */
    bool isValueScalarArray();
    /**
     * @brief Return the interface to the value field.
     * @return The interface. an excetion is thrown if a value field does not exist.
     */
    epics::pvData::PVFieldPtr getValue();
    /**
     * @brief Return the interface to a scalar value field.
     * @return The interface for a scalar value field.
     * An exception is thown if no scalar value field.
     */
    epics::pvData::PVScalarPtr getScalarValue();
    /**
     * @brief Return the interface to an array value field.
     * @return The interface.
     * An exception is thown if no array value field.
     */
    std::tr1::shared_ptr<epics::pvData::PVArray> getArrayValue();
    /**
     * @brief Return the interface to a scalar array value field.
     * @return Return the interface.
     * An exception is thown if no scalar array value field.
     */
    std::tr1::shared_ptr<epics::pvData::PVScalarArray> getScalarArrayValue();
    /**
     * @brief Get the value as a double.
     * If value is not a numeric scalar an exception is thrown.
     * @return The value.
     */
    double getDouble();
    /**
     * @brief Get the value as a string.
     * If value is not a scalar an exception is thrown
     * @return The value.
     */
    std::string getString();
    /**
     * @brief Get the value as a double array.
     * If the value is not a numeric array an exception is thrown.
     * @return The value.
     */
    epics::pvData::shared_vector<const double>  getDoubleArray();
    /**
     * @brief Get the value as a string array.
     * If the value is not a string array an exception is thrown.
     * @return The value.
     */
    epics::pvData::shared_vector<const std::string>  getStringArray();
    /**
     * @brief Get the alarm.
     * If the pvStructure as an alarm field it's values are returned.
     * If no then alarm shows that not alarm defined.
     * @return The alarm.
     */
    epics::pvData::Alarm getAlarm();
    /**
     * @brief Get the timeStamp.
     * If the pvStructure as a timeStamp field, it's values are returned.
     * If no then all fields are 0.
     * @return The timeStamp.
     */
    epics::pvData::TimeStamp getTimeStamp();
private:
    PvaClientGetData(epics::pvData::StructureConstPtr const & structure);
    void checkValue();
    epics::pvData::StructureConstPtr structure;
    epics::pvData::PVStructurePtr pvStructure;
    epics::pvData::BitSetPtr bitSet;

    std::string messagePrefix;
    epics::pvData::PVFieldPtr pvValue;
    epics::pvData::PVAlarm pvAlarm;
    epics::pvData::PVTimeStamp pvTimeStamp;
};

class PvaClientPostHandlerPvt; // private to PvaClientPutData
/**
 * @brief This is a class that holds data given to  by PvaClientPut or PvaClientPutGet
 *
 */
class epicsShareClass PvaClientPutData
{
public:
    POINTER_DEFINITIONS(PvaClientPutData);
    /**
     * @brief Factory method for creating an instance of PvaClientPutData.
     */
    static PvaClientPutDataPtr create(epics::pvData::StructureConstPtr const & structure);
    ~PvaClientPutData() {}
    /**
     * @brief Set a prefix for throw messages.
     * @param value The prefix.
     */
    void setMessagePrefix(std::string const & value);
   /** @brief Get the structure.
    * @return the structure.
    */
   epics::pvData::StructureConstPtr getStructure();
    /** @brief Get the pvStructure.
     * @return the pvStructure.
     */
    epics::pvData::PVStructurePtr getPVStructure();
    /** @brief Get the BitSet for the pvStructure
     * This shows which fields have changed value.
     * @return The bitSet
     */
    epics::pvData::BitSetPtr getBitSet();
    /** @brief show the fields that have changed.
     * @param out The stream that shows the changed fields.
     * @return The stream that was input
     */
    std::ostream & showChanged(std::ostream & out);
    /**
     * @brief Is there a top level field named value.
     * @return The answer.
     */
    bool hasValue();
    /**
     * @brief Is the value field a scalar?
     * @return The answer.
     */
    bool isValueScalar();
    /**
     * @brief Is the value field a scalar array?
     * @return The answer.
     */
    bool isValueScalarArray();
    /**
     * @brief Return the interface to the value field.
     * @return The interface. an excetion is thrown if a value field does not exist.
     */
    epics::pvData::PVFieldPtr getValue();
    /**
     * @brief Return the interface to a scalar value field.
     * @return The interface for a scalar value field.
     * An exception is thown if no scalar value field.
     */
    epics::pvData::PVScalarPtr getScalarValue();
    /**
     * @brief Return the interface to an array value field.
     * @return The interface.
     * An exception is thown if no array value field.
     */
    std::tr1::shared_ptr<epics::pvData::PVArray> getArrayValue();
    /**
     * @brief Return the interface to a scalar array value field.
     * @return Return the interface.
     * An exception is thown if no scalar array value field.
     */
    std::tr1::shared_ptr<epics::pvData::PVScalarArray> getScalarArrayValue();
    /**
     * @brief Get the value as a double.
     * If value is not a numeric scalar an exception is thrown.
     * @return The value.
     */
    double getDouble();
    /**
     * @brief Get the value as a string.
     * If value is not a string an exception is thrown
     * @return The value.
     */
    std::string getString();
    /**
     * @brief Get the value as a double array.
     * If the value is not a numeric array an exception is thrown.
     * @return The value.
     */
    epics::pvData::shared_vector<const double>  getDoubleArray();
    /**
     * @brief Get the value as a string array.
     * If the value is not a string array an exception is thrown.
     * @return The value.
     */
    epics::pvData::shared_vector<const std::string>  getStringArray();
    /**
     * Put the value as a double.
     * An exception is also thrown if the actualy type can cause an overflow.
     * If value is not a numeric scalar an exception is thrown.
     */
    void putDouble(double value);
    /**
     * Put the value as a string.
     * If value is not a  scalar an exception is thrown.
     */
    void putString(std::string const & value);
    /**
     * Copy the array to the value field.
     * If the value field is not a double array field an exception is thrown.
     * @param value The place where data is copied.
     */
    void putDoubleArray(epics::pvData::shared_vector<const double> const & value);
    /**
     * Copy array to the value field.
     * If the value field is not a string array field an exception is thrown.
     * @param value data source
     */
    void putStringArray(epics::pvData::shared_vector<const std::string> const & value);
    /**
     * Copy array to the value field.
     * If the value field is not a scalarArray field an exception is thrown.
     * @param value data source
     */
    void putStringArray(std::vector<std::string> const & value);
private:
    PvaClientPutData(epics::pvData::StructureConstPtr const &structure);
    void checkValue();
    void postPut(size_t fieldNumber);

    std::vector<epics::pvData::PostHandlerPtr> postHandler;
    epics::pvData::StructureConstPtr structure;
    epics::pvData::PVStructurePtr pvStructure;
    epics::pvData::BitSetPtr bitSet;
    friend class PvaClientPostHandlerPvt;

    std::string messagePrefix;
    epics::pvData::PVFieldPtr pvValue;
};

/**
 * @brief This is a class that holds data returned by PvaClientMonitor
 *
 */
class epicsShareClass PvaClientMonitorData
{
public:
    POINTER_DEFINITIONS(PvaClientMonitorData);
    /**
     * @brief Factory method for creating an instance of PvaClientMonitorData.
     */
    static PvaClientMonitorDataPtr create(epics::pvData::StructureConstPtr const & structure);
    ~PvaClientMonitorData() {}
    /**
     * @brief Set a prefix for throw messages.
     * @param value The prefix.
     */
    void setMessagePrefix(std::string const & value);
   /** @brief Get the structure.
    * @return the structure.
    */
   epics::pvData::StructureConstPtr getStructure();
    /** @brief Get the pvStructure.
     * @return the pvStructure.
     */
    epics::pvData::PVStructurePtr getPVStructure();
    /** @brief Get the BitSet for the pvStructure
     * This shows which fields have changed value.
     * @return The bitSet
     */
    epics::pvData::BitSetPtr getChangedBitSet();
    /** @brief Get the overrun BitSet for the pvStructure
     * This shows which fields have had more than one change.
     * @return The bitSet
     */
    epics::pvData::BitSetPtr getOverrunBitSet();
    /** @brief show the fields that have changed.
     * @param out The stream that shows the changed fields.
     * @return The stream that was input
     */
    std::ostream & showChanged(std::ostream & out);
    /** @brief show the fields that have overrun.
     * @param out The stream that shows the overrun fields.
     * @return The stream that was input
     */
    std::ostream & showOverrun(std::ostream & out);
    /**
     * @brief New data is present.
     * @param monitorElement The new data.
     */
    void setData(epics::pvData::MonitorElementPtr const & monitorElement);
    /**
     * @brief Is there a top level field named value.
     * @return The answer.
     */
    bool hasValue();
    /**
     * @brief Is the value field a scalar?
     * @return The answer.
     */
    bool isValueScalar();
    /**
     * @brief Is the value field a scalar array?
     * @return The answer.
     */
    bool isValueScalarArray();
    /**
     * @brief Return the interface to the value field.
     * @return The interface. an excetion is thrown if a value field does not exist.
     */
    epics::pvData::PVFieldPtr getValue();
    /**
     * @brief Return the interface to a scalar value field.
     * @return The interface for a scalar value field.
     * An exception is thown if no scalar value field.
     */
    epics::pvData::PVScalarPtr getScalarValue();
    /**
     * @brief Return the interface to an array value field.
     * @return The interface.
     * An exception is thown if no array value field.
     */
    std::tr1::shared_ptr<epics::pvData::PVArray> getArrayValue();
    /**
     * @brief Return the interface to a scalar array value field.
     * @return Return the interface.
     * An exception is thown if no scalar array value field.
     */
    std::tr1::shared_ptr<epics::pvData::PVScalarArray> getScalarArrayValue();
    /**
     * @brief Get the value as a double.
     * If value is not a numeric scalar an exception is thrown.
     * @return The value.
     */
    double getDouble();
    /**
     * @brief Get the value as a string.
     * If value is not a scalar an exception is thrown
     * @return The value.
     */
    std::string getString();
    /**
     * @brief Get the value as a double array.
     * If the value is not a numeric array an exception is thrown.
     * @return The value.
     */
    epics::pvData::shared_vector<const double>  getDoubleArray();
    /**
     * @brief Get the value as a string array.
     * If the value is not a string array an exception is thrown.
     * @return The value.
     */
    epics::pvData::shared_vector<const std::string>  getStringArray();
    /**
     * @brief Get the alarm.
     * If the pvStructure as an alarm field it's values are returned.
     * If no then alarm shows that not alarm defined.
     * @return The alarm.
     */
    epics::pvData::Alarm getAlarm();
    /**
     * @brief Get the timeStamp.
     * If the pvStructure as a timeStamp field, it's values are returned.
     * If no then all fields are 0.
     * @return The timeStamp.
     */
    epics::pvData::TimeStamp getTimeStamp();
private:
    PvaClientMonitorData(epics::pvData::StructureConstPtr const & structure);
    void checkValue();

    epics::pvData::StructureConstPtr structure;
    epics::pvData::PVStructurePtr pvStructure;
    epics::pvData::BitSetPtr changedBitSet;
    epics::pvData::BitSetPtr overrunBitSet;

    std::string messagePrefix;
    epics::pvData::PVFieldPtr pvValue;
    epics::pvData::PVAlarm pvAlarm;
    epics::pvData::PVTimeStamp pvTimeStamp;
};

class ChannelProcessRequesterImpl; // private to PvaClientProcess
/**
 * @brief An easy to use alternative to ChannelProcess.
 *
 * @author mrk
 */
class epicsShareClass PvaClientProcess 
{
public:
    POINTER_DEFINITIONS(PvaClientProcess);
    /**
     * @brief Create a PvaClientProcess.
     * @param &pvaClient Interface to PvaClient
     * @param pvaClientChannel Interface to PvaClientChannel
     * @param channel Interface to Channel
     * @param pvRequest The request structure.
     * @return The interface to the PvaClientStructure.
     */
    static PvaClientProcessPtr create(
        PvaClientPtr const &pvaClient,
        PvaClientChannelPtr const & pvaClientChannel,
        epics::pvAccess::Channel::shared_pointer const & channel,
        epics::pvData::PVStructurePtr const &pvRequest
    );
    /**
     * @brief destructor
     */
    ~PvaClientProcess();
    /** 
     * @brief destroy an resources used.
     */
    void destroy();
    /**
     * @brief call issueConnect and then waitConnect.
     * An exception is thrown if connect fails.
     */
    void connect();
    /**
     * @brief create the channelProcess connection to the channel.
     * This can only be called once.
     */
    void issueConnect();
    /**
     * @brief wait until the channelProcess connection to the channel is complete.
     * @return status;
     */
    epics::pvData::Status waitConnect();
    /**
     * @brief Call issueProcess and then waitProcess.
     * An exception is thrown if get fails.
     */
    void process();
    /**
     * @brief Issue a get and return immediately.
     */
    void issueProcess();
    /**
     * @brief Wait until get completes.
     * @return status.
     */
    epics::pvData::Status waitProcess();
private:
    PvaClientProcess(
        PvaClientPtr const &pvaClient,
        PvaClientChannelPtr const & pvaClientChannel,
        epics::pvAccess::Channel::shared_pointer const & channel,
        epics::pvData::PVStructurePtr const &pvRequest);
    std::string getRequesterName();
    void message(std::string const & message,epics::pvData::MessageType messageType);
    void channelProcessConnect(
        const epics::pvData::Status& status,
        epics::pvAccess::ChannelProcess::shared_pointer const & channelProcess);
    void processDone(
        const epics::pvData::Status& status,
        epics::pvAccess::ChannelProcess::shared_pointer const & channelProcess);
    void checkProcessState();
    enum ProcessConnectState {connectIdle,connectActive,connected};

    PvaClient::weak_pointer pvaClient;
    PvaClientChannel::weak_pointer pvaClientChannel;
    epics::pvAccess::Channel::shared_pointer channel;
    epics::pvAccess::ChannelProcessRequester::shared_pointer processRequester;
    epics::pvData::PVStructurePtr pvRequest;
    epics::pvData::Mutex mutex;
    epics::pvData::Event waitForConnect;
    epics::pvData::Event waitForProcess;
    std::string messagePrefix;

    bool isDestroyed;
    epics::pvData::Status channelProcessConnectStatus;
    epics::pvData::Status channelProcessStatus;
    epics::pvAccess::ChannelProcess::shared_pointer channelProcess;

    ProcessConnectState connectState;

    enum ProcessState {processIdle,processActive,processComplete};
    ProcessState processState;
    friend class ChannelProcessRequesterImpl;
};

class ChannelGetRequesterImpl; // private to PvaClientGet
/**
 * @brief An easy to use alternative to ChannelGet.
 *
 * @author mrk
 */
class epicsShareClass PvaClientGet 
{
public:
    POINTER_DEFINITIONS(PvaClientGet);
    /**
     * @brief Create a PvaClientGet.
     * @param &pvaClient Interface to PvaClient
     * @param pvaClientChannel Interface to PvaClientChannel
     * @param channel Interface to Channel
     * @param pvRequest The request structure.
     * @return The interface to the PvaClientStructure.
     */
    static PvaClientGetPtr create(
        PvaClientPtr const &pvaClient,
        PvaClientChannelPtr const & pvaClientChannel,
        epics::pvAccess::Channel::shared_pointer const & channel,
        epics::pvData::PVStructurePtr const &pvRequest
    );
    /**
     * @brief destructor
     */
    ~PvaClientGet();
    /** 
     * @brief destroy an resources used.
     */
    void destroy();
    /**
     * @brief call issueConnect and then waitConnect.
     * An exception is thrown if connect fails.
     */
    void connect();
    /**
     * @brief create the channelGet connection to the channel.
     * This can only be called once.
     */
    void issueConnect();
    /**
     * @brief wait until the channelGet connection to the channel is complete.
     * @return status;
     */
    epics::pvData::Status waitConnect();
    /**
     * @brief Call issueGet and then waitGet.
     * An exception is thrown if get fails.
     */
    void get();
    /**
     * @brief Issue a get and return immediately.
     */
    void issueGet();
    /**
     * @brief Wait until get completes.
     * @return status;
     */
    epics::pvData::Status waitGet();
    /**
     * @brief Get the data/
     * @return The interface.
     */
    PvaClientGetDataPtr getData();   
private:
    PvaClientGet(
        PvaClientPtr const &pvaClient,
        PvaClientChannelPtr const & pvaClientChannel,
        epics::pvAccess::Channel::shared_pointer const & channel,
        epics::pvData::PVStructurePtr const &pvRequest);
    std::string getRequesterName();
    void message(std::string const & message,epics::pvData::MessageType messageType);
    void channelGetConnect(
        const epics::pvData::Status& status,
        epics::pvAccess::ChannelGet::shared_pointer const & channelGet,
        epics::pvData::StructureConstPtr const & structure);
    void getDone(
        const epics::pvData::Status& status,
        epics::pvAccess::ChannelGet::shared_pointer const & channelGet,
        epics::pvData::PVStructurePtr const & pvStructure,
        epics::pvData::BitSetPtr const & bitSet);
    void checkGetState();
    enum GetConnectState {connectIdle,connectActive,connected};

    PvaClient::weak_pointer pvaClient;
    PvaClientChannel::weak_pointer pvaClientChannel;
    epics::pvAccess::Channel::shared_pointer channel;
    epics::pvAccess::ChannelGetRequester::shared_pointer getRequester;
    epics::pvData::PVStructurePtr pvRequest;
    epics::pvData::Mutex mutex;
    epics::pvData::Event waitForConnect;
    epics::pvData::Event waitForGet;
    PvaClientGetDataPtr pvaClientData;
    std::string messagePrefix;

    bool isDestroyed;
    epics::pvData::Status channelGetConnectStatus;
    epics::pvData::Status channelGetStatus;
    epics::pvAccess::ChannelGet::shared_pointer channelGet;

    GetConnectState connectState;

    enum GetState {getIdle,getActive,getComplete};
    GetState getState;
    friend class ChannelGetRequesterImpl;
};

class ChannelPutRequesterImpl; // private to PvaClientPut
/**
 * @brief An easy to use alternative to ChannelPut.
 *
 * @author mrk
 */
class epicsShareClass PvaClientPut 
{
public:
    POINTER_DEFINITIONS(PvaClientPut);
    /**
     * @brief Create a PvaClientPut.
     * @param &pvaClient Interface to PvaClient
     * @param pvaClientChannel Interface to PvaClientChannel
     * @param channel Interface to Channel
     * @param pvRequest The request structure.
     * @return The interface to the PvaClientStructure.
     */
    static PvaClientPutPtr create(
        PvaClientPtr const &pvaClient,
        PvaClientChannelPtr const & pvaClientChannel,
        epics::pvAccess::Channel::shared_pointer const & channel,
        epics::pvData::PVStructurePtr const &pvRequest
    );
    /**
     * @brief destructor
     */
    ~PvaClientPut();
    /** 
     * @brief destroy an resources used.
     */
    void destroy();
    /**
     * @brief call issueConnect and then waitConnect.
     * An exception is thrown if connect fails.
     */
    void connect();
    /**
     * @brief create the channelPut connection to the channel.
     * This can only be called once.
     */
    void issueConnect();
    /**
     * @brief wait until the channelPut connection to the channel is complete.
     * @return status;
     */
    epics::pvData::Status waitConnect();
    /**
     * @brief Call issueGet and then waitGet.
     * An exception is thrown if get fails.
     */
    void get();
    /**
     * @brief Issue a get and return immediately.
     */
    void issueGet();
    /**
     * @brief Wait until get completes.
     * @return status
     */
    epics::pvData::Status waitGet();
    /**
     * @brief Call issuePut and then waitPut.
     * An exception is thrown if get fails.
     */
    void put();
    /**
     * @brief Issue a put and return immediately.
     */
    void issuePut();
    /**
     * @brief Wait until put completes.
     * @return status
     */
    epics::pvData::Status waitPut();
    /**
     * @brief Get the data/
     * @return The interface.
     */
    PvaClientPutDataPtr getData();   
private :
    PvaClientPut(
        PvaClientPtr const &pvaClient,
        PvaClientChannelPtr const & pvaClientChannel,
        epics::pvAccess::Channel::shared_pointer const & channel,
        epics::pvData::PVStructurePtr const &pvRequest);
    std::string getRequesterName();
    void message(std::string const & message,epics::pvData::MessageType messageType);
    void channelPutConnect(
        const epics::pvData::Status& status,
        epics::pvAccess::ChannelPut::shared_pointer const & channelPut,
        epics::pvData::StructureConstPtr const & structure);
    void getDone(
        const epics::pvData::Status& status,
        epics::pvAccess::ChannelPut::shared_pointer const & channelPut,
        epics::pvData::PVStructurePtr const & pvStructure,
        epics::pvData::BitSetPtr const & bitSet);
    void putDone(
        const epics::pvData::Status& status,
        epics::pvAccess::ChannelPut::shared_pointer const & channelPut);
    void checkPutState();
    enum PutConnectState {connectIdle,connectActive,connected};

    PvaClient::weak_pointer pvaClient;
    PvaClientChannel::weak_pointer pvaClientChannel;
    epics::pvAccess::Channel::shared_pointer channel;
    epics::pvAccess::ChannelPutRequester::shared_pointer putRequester;
    epics::pvData::PVStructurePtr pvRequest;
    epics::pvData::Mutex mutex;
    epics::pvData::Event waitForConnect;
    epics::pvData::Event waitForGetPut;
    PvaClientPutDataPtr pvaClientData;
    std::string messagePrefix;

    bool isDestroyed;
    epics::pvData::Status channelPutConnectStatus;
    epics::pvData::Status channelGetPutStatus;
    epics::pvAccess::ChannelPut::shared_pointer channelPut;

    PutConnectState connectState;

    enum PutState {putIdle,getActive,putActive,putComplete};
    PutState putState;
    friend class ChannelPutRequesterImpl;
};

class ChannelPutGetRequesterImpl; // private to PvaClientPutGet
/**
 * @brief An easy to use alternative to ChannelPutGet.
 *
 * @author mrk
 */
class epicsShareClass PvaClientPutGet 
{
public:
    POINTER_DEFINITIONS(PvaClientPutGet);
    /**
     * @brief Create a PvaClientPutGet.
     * @param &pvaClient Interface to PvaClient
     * @param pvaClientChannel Interface to PvaClientChannel
     * @param channel Interface to Channel
     * @param pvRequest The request structure.
     * @return The interface to the PvaClientStructure.
     */
    static PvaClientPutGetPtr create(
        PvaClientPtr const &pvaClient,
        PvaClientChannelPtr const & pvaClientChannel,
        epics::pvAccess::Channel::shared_pointer const & channel,
        epics::pvData::PVStructurePtr const &pvRequest
    );
    /**
     * @brief destructor
     */
    ~PvaClientPutGet();
    /** 
     * @brief destroy an resources used.
     */
    void destroy();
    /**
     * @brief call issueConnect and then waitConnect.
     * An exception is thrown if connect fails.
     */
    void connect();
    /**
     * @brief create the channelPutGet connection to the channel.
     * This can only be called once.
     * An exception is thrown if connect fails.
     */
    void issueConnect();
    /**
     * @brief wait until the channelPutGet connection to the channel is complete.
     * @return status;
     */
    epics::pvData::Status waitConnect();
    /**
     * @brief Call issuePutGet and then waitPutGet.
     * An exception is thrown if putGet fails.
     */
    void putGet();
    /**
     * @brief Issue a putGet and return immediately.
     */
    void issuePutGet();
    /**
     * @brief Wait until putGet completes.
     * If failure getStatus can be called to get reason.
     * @return status
     */
    epics::pvData::Status waitPutGet();
    /**
     * @brief Call issueGet and then waitGetGet.
     * An exception is thrown if get fails.
     */
    void getGet();
    /**
     * @brief Issue a getGet and return immediately.
     */
    void issueGetGet();
    /**
     * @brief Wait until getGet completes.
     * If failure getStatus can be called to get reason.
     * @return status
     */
    epics::pvData::Status waitGetGet();
    /**
     * @brief Call issuePut and then waitGetPut.
     * An exception is thrown if getPut fails.
     */
    void getPut();
    /**
     * @brief Issue a getPut and return immediately.
     */
    void issueGetPut();
    /**
     * @brief Wait until getPut completes.
     * @return status
     */
    epics::pvData::Status waitGetPut();
    /**
     * @brief Get the put data.
     * @return The interface.
     */
    PvaClientPutDataPtr getPutData();   
    /**
     * @brief Get the get data.
     * @return The interface.
     */
    PvaClientGetDataPtr getGetData();   
private :
    PvaClientPutGet(
        PvaClientPtr const &pvaClient,
        PvaClientChannelPtr const & pvaClientChannel,
        epics::pvAccess::Channel::shared_pointer const & channel,
        epics::pvData::PVStructurePtr const &pvRequest);
    std::string getRequesterName();
    void message(std::string const & message,epics::pvData::MessageType messageType);
    void channelPutGetConnect(
        const epics::pvData::Status& status,
        epics::pvAccess::ChannelPutGet::shared_pointer const & channelPutGet,
        epics::pvData::StructureConstPtr const & putStructure,
        epics::pvData::StructureConstPtr const & getStructure);
    void putGetDone(
        const epics::pvData::Status& status,
        epics::pvAccess::ChannelPutGet::shared_pointer const & channelPutGet,
        epics::pvData::PVStructurePtr const & getPVStructure,
        epics::pvData::BitSetPtr const & getBitSet);
    void getPutDone(
        const epics::pvData::Status& status,
        epics::pvAccess::ChannelPutGet::shared_pointer const & channelPutGet,
        epics::pvData::PVStructurePtr const & putPVStructure,
        epics::pvData::BitSet::shared_pointer const & putBitSet);
    void getGetDone(
        const epics::pvData::Status& status,
        epics::pvAccess::ChannelPutGet::shared_pointer const & channelPutGet,
        epics::pvData::PVStructurePtr const & getPVStructure,
        epics::pvData::BitSet::shared_pointer const & getBitSet);
    void checkPutGetState();
    enum PutGetConnectState {connectIdle,connectActive,connected};

    PvaClient::weak_pointer pvaClient;
    PvaClientChannel::weak_pointer pvaClientChannel;
    epics::pvAccess::Channel::shared_pointer channel;
    epics::pvAccess::ChannelPutGetRequester::shared_pointer putGetRequester;
    epics::pvData::PVStructurePtr pvRequest;
    epics::pvData::Mutex mutex;
    epics::pvData::Event waitForConnect;
    epics::pvData::Event waitForPutGet;
    PvaClientGetDataPtr pvaClientGetData;
    PvaClientPutDataPtr pvaClientPutData;
    std::string messagePrefix;

    bool isDestroyed;
    epics::pvData::Status channelPutGetConnectStatus;
    epics::pvData::Status channelGetPutGetStatus;
    epics::pvAccess::ChannelPutGet::shared_pointer channelPutGet;

    PutGetConnectState connectState;
    epics::pvData::Status channelPutGetStatus;

    enum PutGetState {putGetIdle,putGetActive,putGetComplete};
    PutGetState putGetState;
    friend class ChannelPutGetRequesterImpl;
};

class ChannelMonitorRequester; // private to PvaClientMonitor
/**
 * @brief Optional client callback.
 *
 */
class epicsShareClass PvaClientMonitorRequester
{
public:
    POINTER_DEFINITIONS(PvaClientMonitorRequester);
    /**
     * @brief destructor
     */
    virtual ~PvaClientMonitorRequester(){}
    /**
     * @brief A monitor event has occurred.
     * @param monitor The PvaClientMonitor that received the event.
     */
    virtual void event(PvaClientMonitorPtr monitor) = 0;
};

/**
 * @brief An easy to use alternative to Monitor.
 *
 */
class epicsShareClass PvaClientMonitor :
     public std::tr1::enable_shared_from_this<PvaClientMonitor>
{
public:
    POINTER_DEFINITIONS(PvaClientMonitor);
    /**
     * @brief Create a PvaClientMonitor.
     * @param &pvaClient Interface to PvaClient
     * @param pvaClientChannel Interface to PvaClientChannel
     * @param channel Interface to Channel
     * @param pvRequest The request structure.
     * @return The interface to the PvaClientStructure.
     */
    static PvaClientMonitorPtr create(
        PvaClientPtr const &pvaClient,
        PvaClientChannelPtr const & pvaClientChannel,
        epics::pvAccess::Channel::shared_pointer const & channel,
        epics::pvData::PVStructurePtr const &pvRequest
    );
    /**
     * @brief destructor
     */
    ~PvaClientMonitor();
    /** 
     * @brief destroy an resources used.
     */
    void destroy();
    /**
     * @brief call issueConnect and then waitConnect.
     * An exception is thrown if connect fails.
     */
    void connect();
    /**
     * @brief create the channelMonitor connection to the channel.
     * This can only be called once.
     * An exception is thrown if connect fails.
     */
    void issueConnect();
    /**
     * @brief wait until the channelMonitor connection to the channel is complete.
     * @return status;
     */
    epics::pvData::Status waitConnect();
    /**
     * @brief Set a user callback.
     * @param pvaClientMonitorrRequester The requester which must be implemented by the caller.
     */
    void setRequester(PvaClientMonitorRequesterPtr const & pvaClientMonitorrRequester);
    /**
     * @brief Start monitoring.
     */
    void start();
    /**
     * @brief Stop monitoring.
     */
    void stop();
    /**
     * @brief poll for a monitor event.
     * The data will be in PvaClientData.
     * @return (false,true) means event (did not, did) occur.
     */
    bool poll();
    /**
     * @brief wait for a monitor event.
     * The data will be in PvaClientData.
     * @param secondsToWait Time to wait for event.
     * @return (false,true) means event (did not, did) occur.
     */
    bool waitEvent(double secondsToWait = 0.0);
    /**
     * @brief Release the monitorElement returned by poll
     */
    void releaseEvent();
    /**
     * @brief The data in which monitor events are placed.
     * @return The interface.
     */
    PvaClientMonitorDataPtr getData();   
    /**
     * @brief get shared pointer to this
     */
    PvaClientMonitorPtr getPtrSelf()
    {
        return shared_from_this();
    }
private:
    PvaClientMonitor(
        PvaClientPtr const &pvaClient,
        PvaClientChannelPtr const & pvaClientChannel,
        epics::pvAccess::Channel::shared_pointer const & channel,
        epics::pvData::PVStructurePtr const &pvRequest);
    std::string getRequesterName();
    void message(std::string const & message,epics::pvData::MessageType messageType);
    void monitorConnect(
        const epics::pvData::Status& status,
        epics::pvData::MonitorPtr const & monitor,
        epics::pvData::StructureConstPtr const & structure);
    void monitorEvent(epics::pvData::MonitorPtr const & monitor);
    void unlisten();
    void checkMonitorState();
    enum MonitorConnectState {connectIdle,connectActive,connected,monitorStarted};

    PvaClient::weak_pointer pvaClient;
    PvaClientChannel::weak_pointer pvaClientChannel;
    epics::pvAccess::Channel::shared_pointer channel;
    epics::pvData::PVStructurePtr pvRequest;
    epics::pvData::MonitorRequester::shared_pointer monitorRequester;
    epics::pvData::Mutex mutex;
    epics::pvData::Event waitForConnect;
    epics::pvData::Event waitForEvent;
    PvaClientMonitorDataPtr pvaClientData;
    std::string messagePrefix;

    bool isDestroyed;
    epics::pvData::Status connectStatus;
    epics::pvData::MonitorPtr monitor;
    epics::pvData::MonitorElementPtr monitorElement;
    PvaClientMonitorRequester::weak_pointer pvaClientMonitorRequester;

    MonitorConnectState connectState;
    bool userPoll;
    bool userWait;
    friend class ChannelMonitorRequester;
};

/**
 * @brief Provides access to multiple channels.
 *
 * @author mrk
 */
class epicsShareClass PvaClientMultiChannel :
    public std::tr1::enable_shared_from_this<PvaClientMultiChannel>
{
public:
    POINTER_DEFINITIONS(PvaClientMultiChannel);
    /**
     * @brief Create a PvaClientMultiChannel.
     * @param channelNames The name. of the channel..
     * @param providerName The name of the provider.
     * @return The interface to the PvaClientStructure.
     */
    static PvaClientMultiChannelPtr create(
         PvaClientPtr const &pvaClient,
         epics::pvData::PVStringArrayPtr const & channelNames,
         std::string const & providerName = "pva");
    ~PvaClientMultiChannel();
    /**
     * @brief Destroy the pvAccess connection.
     */
    void destroy();
    /**
     * @brief Get the channelNames.
     * @return The names.
     */
    epics::pvData::PVStringArrayPtr getChannelNames();
    /**
     * @brief Connect to the channel.
     * This calls issueConnect and waitConnect.
     * An exception is thrown if connect fails.
     * @param timeout The time to wait for connecting to the channel.
     * @param maxNotConnected Maximum number of channels that do not connect.
     * @return status of request
     */
    epics::pvData::Status connect(
       double timeout=5,
       size_t maxNotConnected=0);
    /**
     * Are all channels connected?
     * @return if all are connected.
     */
    bool allConnected();
    /**
     * Has a connection state change occured?
     * @return (true, false) if (at least one, no) channel has changed state.
     */
    bool connectionChange();
    /**
     * Get the connection state of each channel.
     * @return The state of each channel.
     */
    epics::pvData::PVBooleanArrayPtr getIsConnected();
    /**
     * Get the pvaClientChannelArray.
     * @return The weak shared pointer.
     */
    PvaClientChannelArrayWPtr getPvaClientChannelArray();
    /**
     * Get pvaClient.
     * @return The weak shared pointer.
     */
    PvaClient::weak_pointer getPvaClient();
    /**
     * Get the shared pointer to self.
     * @return The shared pointer.
     */
    PvaClientMultiChannelPtr getPtrSelf()
    {
        return shared_from_this();
    }
private:
    PvaClientMultiChannel(
        PvaClientPtr const &pvaClient,
        epics::pvData::PVStringArrayPtr const & channelName,
        std::string const & providerName);

    PvaClient::weak_pointer pvaClient;
    epics::pvData::PVStringArrayPtr channelName;
    std::string providerName;
    size_t numChannel;
    epics::pvData::Mutex mutex;

    size_t numConnected;
    PvaClientChannelArrayPtr pvaClientChannelArray;
    epics::pvData::PVBooleanArrayPtr isConnected;
    bool isDestroyed;
};


}}

#endif  /* PVACLIENT_H */

/** @page Overview Documentation
 *
 * <a href = "pvaClientOverview.html">pvaClientOverview.html</a>
 *
 */


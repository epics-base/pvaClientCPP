/* easyPVA.h */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvData is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
/**
 * @author mrk
 * @date 2015.02
 */
#ifndef EASYPVA_H
#define EASYPVA_H

#ifdef epicsExportSharedSymbols
#   define easyPVAEpicsExportSharedSymbols
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

#ifdef easyPVAEpicsExportSharedSymbols
#   define epicsExportSharedSymbols
#	undef easyPVAEpicsExportSharedSymbols
#endif

#include <shareLib.h>


namespace epics { namespace easyPVA { 

class EasyPVA;
typedef std::tr1::shared_ptr<EasyPVA> EasyPVAPtr;
class EasyGetData;
typedef std::tr1::shared_ptr<EasyGetData> EasyGetDataPtr;
class EasyPutData;
typedef std::tr1::shared_ptr<EasyPutData> EasyPutDataPtr;
class EasyMonitorData;
typedef std::tr1::shared_ptr<EasyMonitorData> EasyMonitorDataPtr;
class EasyChannel;
typedef std::tr1::shared_ptr<EasyChannel> EasyChannelPtr;
class EasyField;
typedef std::tr1::shared_ptr<EasyField> EasyFieldPtr;
class EasyProcess;
typedef std::tr1::shared_ptr<EasyProcess> EasyProcessPtr;
class EasyGet;
typedef std::tr1::shared_ptr<EasyGet> EasyGetPtr;
class EasyPut;
typedef std::tr1::shared_ptr<EasyPut> EasyPutPtr;
class EasyPutGet;
typedef std::tr1::shared_ptr<EasyPutGet> EasyPutGetPtr;
class EasyMonitor;
typedef std::tr1::shared_ptr<EasyMonitor> EasyMonitorPtr;
class EasyMonitorRequester;
typedef std::tr1::shared_ptr<EasyMonitorRequester> EasyMonitorRequesterPtr;
class EasyArray;
typedef std::tr1::shared_ptr<EasyArray> EasyArrayPtr;
class EasyRPC;
typedef std::tr1::shared_ptr<EasyRPC> EasyRPCPtr;

class EasyMultiData;
typedef std::tr1::shared_ptr<EasyMultiData> EasyMultiDataPtr;
class EasyMultiChannel;
typedef std::tr1::shared_ptr<EasyMultiChannel> EasyMultiChannelPtr;
class EasyMultiGet;
typedef std::tr1::shared_ptr<EasyMultiGet> EasyMultiGetPtr;
class EasyMultiPut;
typedef std::tr1::shared_ptr<EasyMultiPut> EasyMultiPutPtr;
class EasyMultiMonitor;
typedef std::tr1::shared_ptr<EasyMultiMonitor> EasyMultiMonitorPtr;

// following are private to easyPVA
class EasyChannelCache;
typedef std::tr1::shared_ptr<EasyChannelCache> EasyChannelCachePtr;

/**
 * @brief EasyPVA is an easy to use interface to pvAccess.
 *
 * @author mrk
 */
class epicsShareClass EasyPVA :
     public epics::pvData::Requester,
     public std::tr1::enable_shared_from_this<EasyPVA>
{
public:
    POINTER_DEFINITIONS(EasyPVA);

    /**
     * Destructor
     */
    ~EasyPVA();
    /**
     * @brief Create an instance of EasyPVA
     * @return shared_ptr to new instance.
     */
    static EasyPVAPtr create();
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
     * The provider is pva. The timeout is 5 seconds.
     * If connection can not be made an exception is thrown.
     * @param channelName The channelName.
     * @return The interface.
     */
    EasyChannelPtr channel(std::string const & channelName)
    { return channel(channelName,"pva", 5.0); }
    /**
     * @brief get a cached channel or create and connect to a new channel.
     * If connection can not be made an exception is thrown.
     * @param channelName The channelName.
     * @return The interface.
     */
    EasyChannelPtr channel(
        std::string const & channelName,
        std::string const &providerName,
        double timeOut);
    /**
     * @brief Create an EasyChannel. The provider is pva.
     * @param channelName The channelName.
     * @return The interface.
     */
    EasyChannelPtr createChannel(std::string const & channelName);
    /**
     * @brief Create an EasyChannel with the specified provider.
     * @param channelName The channelName.
     * @param providerName The provider.
     * @return The interface or null if the provider does not exist.
     */
    EasyChannelPtr createChannel(
       std::string const & channelName,
       std::string const & providerName);
    /**
     * @brief Create an EasyMultiChannel. The provider is pvAccess.
     * @param channelName The channelName array.
     * @return The interface.
     */
    EasyMultiChannelPtr createMultiChannel(epics::pvData::StringArray const & channelName);
    /**
     * @brief Create an EasyMultiChannel with the specified provider.
     * @param channelName The channelName array.
     * @param providerName The provider.
     * @return The interface.
     */
    EasyMultiChannelPtr createMultiChannel(
            epics::pvData::StringArray const & channelName,
            std::string const & providerName);
    /**
     * @brief Create an EasyMultiChannel with the specified provider.
     * @param channelName The channelName.
     * @param providerName The provider.
     * @param union The union interface for the value field of each channel.
     * @return The interface.
     */
    EasyMultiChannelPtr createMultiChannel(
            epics::pvData::StringArray const & channelName,
            std::string const & providerName,
            epics::pvData::UnionConstPtr const & u);
    /**
     * @brief Set a requester.
     * The default is for EasyPVA to handle messages by printing to System.out.
     * @param requester The requester.
     */
    void setRequester(epics::pvData::RequesterPtr const & requester);
    /**
     * @brief Clear the requester. EasyPVA will handle messages.
     */
    void clearRequester();
    /**
     * @brief get shared pointer to this
     */
    EasyPVAPtr getPtrSelf()
    {
        return shared_from_this();
    }
private:
    EasyPVA();
    EasyChannelCachePtr easyChannelCache;

    epics::pvData::PVStructurePtr createRequest(std::string const &request);
    std::list<EasyChannelPtr> channelList;
    std::list<EasyMultiChannelPtr> multiChannelList;
    epics::pvData::Requester::weak_pointer requester;
    bool isDestroyed;
    epics::pvData::Mutex mutex;
};

// folowing private to EasyChannel
class EasyGetCache;
typedef std::tr1::shared_ptr<EasyGetCache> EasyGetCachePtr;
class EasyPutCache;
typedef std::tr1::shared_ptr<EasyPutCache> EasyPutCachePtr;
class ChannelRequesterImpl;
/**
 * @brief An easy to use alternative to directly calling the Channel methods of pvAccess.
 *
 * @author mrk
 */
class epicsShareClass EasyChannel :
    public std::tr1::enable_shared_from_this<EasyChannel>
{
public:
    POINTER_DEFINITIONS(EasyChannel);
    /**
     * @brief Create a EasyChannel.
     * @param easyPVA Interface to EasyPVA
     * @param channelName The name of the channel.
     * @return The interface to the EasyPVAStructure.
     */
    static EasyChannelPtr create(
        EasyPVAPtr const &easyPVA,
         std::string const & channelName) {return create(easyPVA,channelName,"pva");}
    /**
     * @brief Create a EasyChannel.
     * @param channelName The name of the channel.
     * @param providerName The name of the provider.
     * @return The interface to the EasyPVAStructure.
     */
    static EasyChannelPtr create(
         EasyPVAPtr const &easyPVA,
         std::string const & channelName,
         std::string const & providerName);
    ~EasyChannel();
    /**
     * @brief Destroy the pvAccess connection.
     */
    void destroy();
    /**
     * @brief Get the name of the channel to which EasyChannel is connected.
     * @return The channel name.
     */
    std::string getChannelName();
    /**
     * @brief Connect to the channel.
     * This calls issueConnect and waitConnect.
     * An exception is thrown if connect fails.
     * @param timeout The time to wait for connecting to the channel.
     */
    void connect(double timeout);
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
    EasyFieldPtr createField();
    /**
     * @brief Create an EasyField for the specified subField.
     * @param subField The syntax for subField is defined in package org.epics.pvdata.copy
     * @return The interface.
     */
    EasyFieldPtr createField(std::string const & subField);
    /**
     * @brief Calls the next method with request = "";
     * @return The interface.
     */
    EasyProcessPtr createProcess();
    /**
     * @brief First call createRequest as implemented by pvDataJava and then calls the next method.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    EasyProcessPtr createProcess(std::string const & request);
    /**
     * @brief Creates an EasyProcess. 
     * @param pvRequest The syntax of pvRequest is described in package org.epics.pvdata.copy.
     * @return The interface.
     */
    EasyProcessPtr createProcess(epics::pvData::PVStructurePtr const &  pvRequest);
    /**
     * @brief Call the next method with request =  "field(value,alarm,timeStamp)" 
     * @return The interface.
     */
    EasyGetPtr get();
    /**
     * @brief get a cached EasyGet or create and connect to a new EasyGet.
     * Then call it's get method.
     * If connection can not be made an exception is thrown.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    EasyGetPtr get(std::string const & request);
    /**
     * @brief Call the next method with request =  "field(value,alarm,timeStamp)" 
     * @return The interface.
     */
    EasyGetPtr createGet();
    /**
     * @brief First call createRequest as implemented by pvDataJava and then call the next method.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    EasyGetPtr createGet(std::string const & request);
    /**
     * @brief Creates an EasyGet.
     * @param pvRequest The syntax of pvRequest is described in package org.epics.pvdata.copy.
     * @return The interface.
     */
    EasyGetPtr createGet(epics::pvData::PVStructurePtr const &  pvRequest);
    /**
     * @brief Call the next method with request =  "field(value)" 
     * @return The interface.
     */
    EasyPutPtr put();
    /**
     *  @brief get a cached EasyPut or create and connect to a new EasyPut.
     *  Then call it's get method.
     *  If connection can not be made an exception is thrown.
     *  @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    EasyPutPtr put(std::string const & request);
    /**
     *  @brief Call the next method with request = "field(value)" 
     * @return The interface.
     */
    EasyPutPtr createPut();
    /**
     * @brief First call createRequest as implemented by pvDataJava and then calls the next method.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    EasyPutPtr createPut(std::string const & request);
    /**
     * @brief Create an EasyPut.
     * @param pvRequest The syntax of pvRequest is described in package org.epics.pvdata.copy.
     * @return The interface.
     */
    EasyPutPtr createPut(epics::pvData::PVStructurePtr const & pvRequest);
    /**
     *  @brief Call the next method with request = "record[process=true]putField(argument)getField(result)".
     * @return The interface.
     */
    EasyPutGetPtr createPutGet();
    /**
     * @brief First call createRequest as implemented by pvDataJava and then calls the next method.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    EasyPutGetPtr createPutGet(std::string const & request);
    /**
     * @brief Create an EasyPutGet.
     * @param pvRequest The syntax of pvRequest is described in package org.epics.pvdata.copy.
     * @return The interface.
     */
    EasyPutGetPtr createPutGet(epics::pvData::PVStructurePtr const & pvRequest);
    /**
     * @brief Call createRPC(PVStructure(null))
     * @return The interface.
     */
    EasyRPCPtr createRPC();
    /**
     * @brief First call createRequest as implemented by pvDataJava and then calls the next method.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    EasyRPCPtr createRPC(std::string const & request);
    /**
     * @brief Create an EasyRPC.
     * @param pvRequest The syntax of pvRequest is described in package org.epics.pvdata.copy.
     * @return The interface.
     */
    EasyRPCPtr createRPC(epics::pvData::PVStructurePtr const & pvRequest);
    /**
     * @brief Call the next method with request = "field(value)";
     * @return The interface.
     */
    EasyArrayPtr createArray();
    /**
     * @brief First call createRequest as implemented by pvDataJava and then calls the next method.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    EasyArrayPtr createArray(std::string const & request);
    /**
     * @brief Create an EasyArray.
     * @param pvRequest The syntax of pvRequest is described in package org.epics.pvdata.copy.
     * @return The interface.
     */
    EasyArrayPtr createArray(epics::pvData::PVStructurePtr const &  pvRequest);
    /**
     * @brief Call the next method with request =  "field(value,alarm,timeStamp)" 
     * @return The interface.
     */
    EasyMonitorPtr monitor();
    /**
     * @brief get a cached EasyMonitor or create and connect to a new EasyMonitor.
     * Then call it's start method.
     * If connection can not be made an exception is thrown.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    EasyMonitorPtr monitor(std::string const & request);
    /**
      * @brief Call the next method with request =  "field(value,alarm,timeStamp)" 
      * @param easyMonitorRequester The client callback.
      * @return The interface.
      */
    EasyMonitorPtr monitor(EasyMonitorRequesterPtr const & easyMonitorRequester);

    /**
     * @brief get a cached EasyMonitor or create and connect to a new EasyMonitor.
     * Then call it's start method.
     * If connection can not be made an exception is thrown.
     * @param request The request as described in package org.epics.pvdata.copy
     * @param easyMonitorRequester The client callback.
     * @return The interface.
     */
    EasyMonitorPtr monitor(
        std::string const & request,
        EasyMonitorRequesterPtr const & easyMonitorRequester);
    /**
     * @brief Call the next method with request = "field(value.alarm,timeStamp)" 
     * @return The interface.
     */
    EasyMonitorPtr createMonitor();
    /**
     * @brief First call createRequest as implemented by pvDataJava and then calls the next method.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    EasyMonitorPtr createMonitor(std::string const & request);
    /**
     * @brief Create an EasyMonitor.
     * @param pvRequest  The syntax of pvRequest is described in package org.epics.pvdata.copy.
     * @return The interface.
     */
    EasyMonitorPtr createMonitor(epics::pvData::PVStructurePtr const &  pvRequest);
    EasyChannelPtr getPtrSelf()
    {
        return shared_from_this();
    }
private:
    EasyChannel(
        EasyPVAPtr const &pva,
        std::string const & channelName,
        std::string const & providerName);
    void channelCreated(
        const epics::pvData::Status& status,
        epics::pvAccess::Channel::shared_pointer const & channel);
    void channelStateChange(
        epics::pvAccess::Channel::shared_pointer const & channel,
        epics::pvAccess::Channel::ConnectionState connectionState);
    std::tr1::shared_ptr<epics::pvAccess::Channel> getChannel();
    std::string getRequesterName();
    void message(
        std::string const & message,
        epics::pvData::MessageType messageType);

    enum ConnectState {connectIdle,connectActive,notConnected,connected};

    EasyPVA::weak_pointer easyPVA;
    std::string channelName;
    std::string providerName;
    ConnectState connectState;
    bool isDestroyed;
    epics::pvData::CreateRequest::shared_pointer createRequest;
    EasyGetCachePtr easyGetCache;
    EasyPutCachePtr easyPutCache;

    epics::pvData::Status channelConnectStatus;
    epics::pvData::Mutex mutex;
    epics::pvData::Event waitForConnect;
    epics::pvAccess::Channel::shared_pointer channel;
    epics::pvAccess::ChannelRequester::shared_pointer channelRequester;
    friend class ChannelRequesterImpl;
};

/**
 * @brief This is a class that holds data returned by EasyGet or EasyPutGet
 *
 */
class epicsShareClass EasyGetData
{
public:
    POINTER_DEFINITIONS(EasyGetData);
    /**
     * @brief Factory method for creating an instance of EasyGetData.
     */
    static EasyGetDataPtr create(epics::pvData::StructureConstPtr const & structure);
    ~EasyGetData() {}
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
    EasyGetData(epics::pvData::StructureConstPtr const & structure);
    void checkValue();
    epics::pvData::StructureConstPtr structure;
    epics::pvData::PVStructurePtr pvStructure;
    epics::pvData::BitSetPtr bitSet;

    std::string messagePrefix;
    epics::pvData::PVFieldPtr pvValue;
    epics::pvData::PVAlarm pvAlarm;
    epics::pvData::PVTimeStamp pvTimeStamp;
};

class EasyPostHandlerPvt; // private to EasyPutData
/**
 * @brief This is a class that holds data given to  by EasyPut or EasyPutGet
 *
 */
class epicsShareClass EasyPutData
{
public:
    POINTER_DEFINITIONS(EasyPutData);
    /**
     * @brief Factory method for creating an instance of EasyPutData.
     */
    static EasyPutDataPtr create(epics::pvData::StructureConstPtr const & structure);
    ~EasyPutData() {}
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
     * Copy the sub-array to the value field.
     * If the value field is not a double array field an exception is thrown.
     * @param value The place where data is copied.
     */
    void putDoubleArray(epics::pvData::shared_vector<const double> const & value);
    /**
     * Copy the sub-array to the value field.
     * If the value field is not a double array field an exception is thrown.
     * @param value The place where data is copied.
     */
    void putStringArray(epics::pvData::shared_vector<const std::string> const & value);
private:
    EasyPutData(epics::pvData::StructureConstPtr const &structure);
    void checkValue();
    void postPut(size_t fieldNumber);

    std::vector<epics::pvData::PostHandlerPtr> postHandler;
    epics::pvData::StructureConstPtr structure;
    epics::pvData::PVStructurePtr pvStructure;
    epics::pvData::BitSetPtr bitSet;
    friend class EasyPostHandlerPvt;

    std::string messagePrefix;
    epics::pvData::PVFieldPtr pvValue;
};

/**
 * @brief This is a class that holds data returned by EasyMonitor
 *
 */
class epicsShareClass EasyMonitorData
{
public:
    POINTER_DEFINITIONS(EasyMonitorData);
    /**
     * @brief Factory method for creating an instance of EasyMonitorData.
     */
    static EasyMonitorDataPtr create(epics::pvData::StructureConstPtr const & structure);
    ~EasyMonitorData() {}
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
    EasyMonitorData(epics::pvData::StructureConstPtr const & structure);
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

class ChannelProcessRequesterImpl; // private to ChannelProcess.
/**
 * @brief An easy to use alternative to ChannelProcess.
 *
 * @author mrk
 */
class epicsShareClass EasyProcess 
{
public:
    POINTER_DEFINITIONS(EasyProcess);
    /**
     * @brief Create a EasyProcess.
     * @param &easyPVA Interface to EasyPVA
     * @param easyChannel Interface to EasyChannel
     * @param channel Interface to Channel
     * @param pvRequest The request structure.
     * @return The interface to the EasyPVAStructure.
     */
    static EasyProcessPtr create(
        EasyPVAPtr const &easyPVA,
        EasyChannelPtr const & easyChannel,
        epics::pvAccess::Channel::shared_pointer const & channel,
        epics::pvData::PVStructurePtr const &pvRequest
    );
    /**
     * @brief destructor
     */
    ~EasyProcess();
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
    EasyProcess(
        EasyPVAPtr const &easyPVA,
        EasyChannelPtr const & easyChannel,
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

    EasyPVA::weak_pointer easyPVA;
    EasyChannel::weak_pointer easyChannel;
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

class ChannelGetRequesterImpl; // private to ChannelGet.
/**
 * @brief An easy to use alternative to ChannelGet.
 *
 * @author mrk
 */
class epicsShareClass EasyGet 
{
public:
    POINTER_DEFINITIONS(EasyGet);
    /**
     * @brief Create a EasyGet.
     * @param &easyPVA Interface to EasyPVA
     * @param easyChannel Interface to EasyChannel
     * @param channel Interface to Channel
     * @param pvRequest The request structure.
     * @return The interface to the EasyPVAStructure.
     */
    static EasyGetPtr create(
        EasyPVAPtr const &easyPVA,
        EasyChannelPtr const & easyChannel,
        epics::pvAccess::Channel::shared_pointer const & channel,
        epics::pvData::PVStructurePtr const &pvRequest
    );
    /**
     * @brief destructor
     */
    ~EasyGet();
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
    EasyGetDataPtr getData();   
private:
    EasyGet(
        EasyPVAPtr const &easyPVA,
        EasyChannelPtr const & easyChannel,
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

    EasyPVA::weak_pointer easyPVA;
    EasyChannel::weak_pointer easyChannel;
    epics::pvAccess::Channel::shared_pointer channel;
    epics::pvAccess::ChannelGetRequester::shared_pointer getRequester;
    epics::pvData::PVStructurePtr pvRequest;
    epics::pvData::Mutex mutex;
    epics::pvData::Event waitForConnect;
    epics::pvData::Event waitForGet;
    EasyGetDataPtr easyData;
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

class ChannelPutRequesterImpl; // private to ChannelPut.
/**
 * @brief An easy to use alternative to ChannelPut.
 *
 * @author mrk
 */
class epicsShareClass EasyPut 
{
public:
    POINTER_DEFINITIONS(EasyPut);
    /**
     * @brief Create a EasyPut.
     * @param &easyPVA Interface to EasyPVA
     * @param easyChannel Interface to EasyChannel
     * @param channel Interface to Channel
     * @param pvRequest The request structure.
     * @return The interface to the EasyPVAStructure.
     */
    static EasyPutPtr create(
        EasyPVAPtr const &easyPVA,
        EasyChannelPtr const & easyChannel,
        epics::pvAccess::Channel::shared_pointer const & channel,
        epics::pvData::PVStructurePtr const &pvRequest
    );
    /**
     * @brief destructor
     */
    ~EasyPut();
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
    EasyPutDataPtr getData();   
private :
    EasyPut(
        EasyPVAPtr const &easyPVA,
        EasyChannelPtr const & easyChannel,
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

    EasyPVA::weak_pointer easyPVA;
    EasyChannel::weak_pointer easyChannel;
    epics::pvAccess::Channel::shared_pointer channel;
    epics::pvAccess::ChannelPutRequester::shared_pointer putRequester;
    epics::pvData::PVStructurePtr pvRequest;
    epics::pvData::Mutex mutex;
    epics::pvData::Event waitForConnect;
    epics::pvData::Event waitForGetPut;
    EasyPutDataPtr easyData;
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

class ChannelPutGetRequesterImpl; // private to ChannelPutGet.
/**
 * @brief An easy to use alternative to ChannelPutGet.
 *
 * @author mrk
 */
class epicsShareClass EasyPutGet 
{
public:
    POINTER_DEFINITIONS(EasyPutGet);
    /**
     * @brief Create a EasyPutGet.
     * @param &easyPVA Interface to EasyPVA
     * @param easyChannel Interface to EasyChannel
     * @param channel Interface to Channel
     * @param pvRequest The request structure.
     * @return The interface to the EasyPVAStructure.
     */
    static EasyPutGetPtr create(
        EasyPVAPtr const &easyPVA,
        EasyChannelPtr const & easyChannel,
        epics::pvAccess::Channel::shared_pointer const & channel,
        epics::pvData::PVStructurePtr const &pvRequest
    );
    /**
     * @brief destructor
     */
    ~EasyPutGet();
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
    EasyPutDataPtr getPutData();   
    /**
     * @brief Get the get data.
     * @return The interface.
     */
    EasyGetDataPtr getGetData();   
private :
    EasyPutGet(
        EasyPVAPtr const &easyPVA,
        EasyChannelPtr const & easyChannel,
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

    EasyPVA::weak_pointer easyPVA;
    EasyChannel::weak_pointer easyChannel;
    epics::pvAccess::Channel::shared_pointer channel;
    epics::pvAccess::ChannelPutGetRequester::shared_pointer putGetRequester;
    epics::pvData::PVStructurePtr pvRequest;
    epics::pvData::Mutex mutex;
    epics::pvData::Event waitForConnect;
    epics::pvData::Event waitForPutGet;
    EasyGetDataPtr easyGetData;
    EasyPutDataPtr easyPutData;
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

class ChannelMonitorRequester; // private to EasyMonitor
/**
 * @brief Optional client callback.
 *
 */
class epicsShareClass EasyMonitorRequester
{
public:
    POINTER_DEFINITIONS(EasyMonitorRequester);
    /**
     * @brief destructor
     */
    virtual ~EasyMonitorRequester(){}
    /**
     * @brief A monitor event has occurred.
     * @param monitor The EasyMonitor that received the event.
     */
    virtual void event(EasyMonitorPtr monitor) = 0;
};

/**
 * @brief An easy to use alternative to Monitor.
 *
 */
class epicsShareClass EasyMonitor :
     public std::tr1::enable_shared_from_this<EasyMonitor>
{
public:
    POINTER_DEFINITIONS(EasyMonitor);
    /**
     * @brief Create a EasyMonitor.
     * @param &easyPVA Interface to EasyPVA
     * @param easyChannel Interface to EasyChannel
     * @param channel Interface to Channel
     * @param pvRequest The request structure.
     * @return The interface to the EasyPVAStructure.
     */
    static EasyMonitorPtr create(
        EasyPVAPtr const &easyPVA,
        EasyChannelPtr const & easyChannel,
        epics::pvAccess::Channel::shared_pointer const & channel,
        epics::pvData::PVStructurePtr const &pvRequest
    );
    /**
     * @brief destructor
     */
    ~EasyMonitor();
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
     * @param easyMonitorrRequester The requester which must be implemented by the caller.
     */
    void setRequester(EasyMonitorRequesterPtr const & easyMonitorrRequester);
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
     * The data will be in EasyData.
     * @return (false,true) means event (did not, did) occur.
     */
    bool poll();
    /**
     * @brief wait for a monitor event.
     * The data will be in EasyData.
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
    EasyMonitorDataPtr getData();   
    /**
     * @brief get shared pointer to this
     */
    EasyMonitorPtr getPtrSelf()
    {
        return shared_from_this();
    }
private:
    EasyMonitor(
        EasyPVAPtr const &easyPVA,
        EasyChannelPtr const & easyChannel,
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

    EasyPVA::weak_pointer easyPVA;
    EasyChannel::weak_pointer easyChannel;
    epics::pvAccess::Channel::shared_pointer channel;
    epics::pvData::PVStructurePtr pvRequest;
    epics::pvData::MonitorRequester::shared_pointer monitorRequester;
    epics::pvData::Mutex mutex;
    epics::pvData::Event waitForConnect;
    epics::pvData::Event waitForEvent;
    EasyMonitorDataPtr easyData;
    std::string messagePrefix;

    bool isDestroyed;
    epics::pvData::Status connectStatus;
    epics::pvData::MonitorPtr monitor;
    epics::pvData::MonitorElementPtr monitorElement;
    EasyMonitorRequester::weak_pointer easyMonitorRequester;

    MonitorConnectState connectState;
    bool userPoll;
    bool userWait;
    friend class ChannelMonitorRequester;
};

}}

#endif  /* EASYPVA_H */

/** @page Overview Documentation
 *
 * <a href = "easyPVA.html">easyPVA.html</a>
 *
 */


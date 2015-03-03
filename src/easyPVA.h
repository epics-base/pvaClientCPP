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
#include <pv/requester.h>
#include <pv/status.h>
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
class EasyPVStructure;
typedef std::tr1::shared_ptr<EasyPVStructure> EasyPVStructurePtr;
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
    virtual ~EasyPVA();
    /**
     * @brief Create an instance of EasyPVA
     * @return shared_ptr to new instance.
     */
    static EasyPVAPtr create();
    /** @brief get the requester name.
     * @return The name.
     */
    virtual std::string getRequesterName();
    /**
     * @brief A new message.
     * If a requester is set then it is called otherwise message is displayed
     * on standard out.
     * @param message The message.
     * @param messageType The type.
     */
    virtual void message(
        std::string const & message,
        epics::pvData::MessageType messageType);
    
    /**
     * @brief Destroy all the channels and multiChannels.
     */
    void destroy();
    /**
     * @brief Create a EasyPVStructure.
     * @return The interface to the EasyPVStructure.
     */
    EasyPVStructurePtr createEasyPVStructure();
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

/**
 * @brief An easy to use alternative to directly calling the Channel methods of pvAccess.
 *
 * @author mrk
 */
class epicsShareClass EasyChannel
{
public:
    POINTER_DEFINITIONS(EasyChannel);
    virtual ~EasyChannel() { }
    /**
     * @brief Destroy the pvAccess connection.
     */
    virtual void destroy() = 0;
    /**
     * @brief Get the name of the channel to which EasyChannel is connected.
     * @return The channel name.
     */
    virtual std::string getChannelName() = 0;
    /**
     * @brief Connect to the channel.
     * This calls issueConnect and waitConnect.
     * An exception is thrown if connect fails.
     * @param timeout The time to wait for connecting to the channel.
     */
    virtual void connect(double timeout) = 0;
    /**
     * @brief Issue a connect request and return immediately.
     */
    virtual void issueConnect() = 0;
    /**
     * @brief Wait until the connection completes or for timeout.
     * @param timeout The time in second to wait.
     * @return status.
     */
    virtual epics::pvData::Status waitConnect(double timeout) = 0;
    /**
     * @brief Calls the next method with subField = "";
     * @return The interface.
     */
    virtual EasyFieldPtr createField() = 0;
    /**
     * @brief Create an EasyField for the specified subField.
     * @param subField The syntax for subField is defined in package org.epics.pvdata.copy
     * @return The interface.
     */
    virtual EasyFieldPtr createField(std::string const & subField) = 0;
    /**
     * @brief Calls the next method with request = "";
     * @return The interface.
     */
    virtual EasyProcessPtr createProcess() = 0;
    /**
     * @brief First call createRequest as implemented by pvDataJava and then calls the next method.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    virtual EasyProcessPtr createProcess(std::string const & request) = 0;
    /**
     * @brief Creates an EasyProcess. 
     * @param pvRequest The syntax of pvRequest is described in package org.epics.pvdata.copy.
     * @return The interface.
     */
    virtual EasyProcessPtr createProcess(epics::pvData::PVStructurePtr const &  pvRequest) = 0;
    /**
     * @brief Call the next method with request =  "field(value,alarm,timeStamp)" 
     * @return The interface.
     */
    virtual EasyGetPtr get() = 0;
    /**
     * @brief get a cached EasyGet or create and connect to a new EasyGet.
     * Then call it's get method.
     * If connection can not be made an exception is thrown.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    virtual EasyGetPtr get(std::string const & request) = 0;
    /**
     * @brief Call the next method with request =  "field(value,alarm,timeStamp)" 
     * @return The interface.
     */
    virtual EasyGetPtr createGet() = 0;
    /**
     * @brief First call createRequest as implemented by pvDataJava and then call the next method.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    virtual EasyGetPtr createGet(std::string const & request) = 0;
    /**
     * @brief Creates an EasyGet.
     * @param pvRequest The syntax of pvRequest is described in package org.epics.pvdata.copy.
     * @return The interface.
     */
    virtual EasyGetPtr createGet(epics::pvData::PVStructurePtr const &  pvRequest) = 0;
    /**
     *  @brief Call the next method with request = "field(value)" 
     * @return The interface.
     */
    virtual EasyPutPtr createPut() = 0;
    /**
     * @brief First call createRequest as implemented by pvDataJava and then calls the next method.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    virtual EasyPutPtr createPut(std::string const & request) = 0;
    /**
     * @brief Create an EasyPut.
     * @param pvRequest The syntax of pvRequest is described in package org.epics.pvdata.copy.
     * @return The interface.
     */
    virtual EasyPutPtr createPut(epics::pvData::PVStructurePtr const & pvRequest) = 0;
    /**
     *  @brief Call the next method with request = "record[process=true]putField(argument)getField(result)".
     * @return The interface.
     */
    virtual EasyPutGetPtr createPutGet() = 0;
    /**
     * @brief First call createRequest as implemented by pvDataJava and then calls the next method.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    virtual EasyPutGetPtr createPutGet(std::string const & request) = 0;
    /**
     * @brief Create an EasyPutGet.
     * @param pvRequest The syntax of pvRequest is described in package org.epics.pvdata.copy.
     * @return The interface.
     */
    virtual EasyPutGetPtr createPutGet(epics::pvData::PVStructurePtr const & pvRequest) = 0;
    /**
     * @brief Call createRPC(PVStructure(null))
     * @return The interface.
     */
    virtual EasyRPCPtr createRPC() = 0;
    /**
     * @brief First call createRequest as implemented by pvDataJava and then calls the next method.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    virtual EasyRPCPtr createRPC(std::string const & request) = 0;
    /**
     * @brief Create an EasyRPC.
     * @param pvRequest The syntax of pvRequest is described in package org.epics.pvdata.copy.
     * @return The interface.
     */
    virtual EasyRPCPtr createRPC(epics::pvData::PVStructurePtr const & pvRequest) = 0;
    /**
     * @brief Call the next method with request = "field(value)";
     * @return The interface.
     */
    virtual EasyArrayPtr createArray() = 0;
    /**
     * @brief First call createRequest as implemented by pvDataJava and then calls the next method.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    virtual EasyArrayPtr createArray(std::string const & request) = 0;
    /**
     * @brief Create an EasyArray.
     * @param pvRequest The syntax of pvRequest is described in package org.epics.pvdata.copy.
     * @return The interface.
     */
    virtual EasyArrayPtr createArray(epics::pvData::PVStructurePtr const &  pvRequest) = 0;
    /**
     * @brief Call the next method with request = "field(value.alarm,timeStamp)" 
     * @return The interface.
     */
    virtual EasyMonitorPtr createMonitor() = 0;
    /**
     * @brief First call createRequest as implemented by pvDataJava and then calls the next method.
     * @param request The request as described in package org.epics.pvdata.copy
     * @return The interface.
     */
    virtual EasyMonitorPtr createMonitor(std::string const & request) = 0;
    /**
     * @brief Create an EasyMonitor.
     * @param pvRequest  The syntax of pvRequest is described in package org.epics.pvdata.copy.
     * @return The interface.
     */
    virtual EasyMonitorPtr createMonitor(epics::pvData::PVStructurePtr const &  pvRequest) = 0;
};

/**
 * @brief This is a factory for creating an EasyChannel
 *
 * @author mrk
 */
class epicsShareClass EasyChannelFactory
{
public:
    /**
     * @brief Create a EasyPVAStructure.
     * @param easyPVA Interface to EasyPVA
     * @param channelName The name of the channel.
     * @return The interface to the EasyPVAStructure.
     */
    static EasyChannelPtr createEasyChannel(
        EasyPVAPtr const &easyPVA,
         std::string const & channelName);
    /**
     * @brief Create a EasyPVAStructure.
     * @param channelName The name of the channel.
     * @param providerName The name of the provider.
     * @return The interface to the EasyPVAStructure.
     */
    static EasyChannelPtr createEasyChannel(
         EasyPVAPtr const &easyPVA,
         std::string const & channelName,
         std::string const & providerName);
};

/**
 * @brief This is a convenience wrapper for a PVStructure.
 *
 * @author mrk
 */
class epicsShareClass EasyPVStructure
{
public:
    POINTER_DEFINITIONS(EasyPVStructure);
    /**
     * @brief Set a prefix to be added to any messages generated by EasyPVStructure.
     * @param value The prefix.
     */
    virtual void setMessagePrefix(std::string const & value) = 0;
    /**
     * @brief Set the pvStructure on which the remaining methods operate.
     * @param pvStructure The structure.
     */
    virtual void setPVStructure(epics::pvData::PVStructurePtr const & pvStructure) = 0;
    /**
     * @brief Get the top level pvStructure.
     * @return The pvStructure. An exception is thrown if pvStructure does not exist.
     */
    virtual epics::pvData::PVStructurePtr getPVStructure() = 0;
    /**
     * @brief Get the alarm.
     * If the pvStructure as an alarm field it's values are returned.
     * If no then alarm shows that not alarm defined.
     * @return The alarm.
     */
    virtual epics::pvData::Alarm getAlarm() = 0;
    /**
     * @brief Get the timeStamp.
     * If the pvStructure as a timeStamp field, it's values are returned.
     * If no then all fields are 0.
     * @return The timeStamp.
     */
    virtual epics::pvData::TimeStamp getTimeStamp() = 0;
    /**
     * @brief Is there a top level field named value of the PVstructure returned by channelGet?
     * @return The answer.
     */
    virtual bool hasValue() = 0;
    /**
     * @brief Is the value field a scalar?
     * @return The answer.
     */
    virtual bool isValueScalar() = 0;
    /**
     * @brief Is the value field a scalar array?
     * @return The answer.
     */
    virtual bool isValueScalarArray() = 0;
    /**
     * @brief Return the interface to the value field.
     * @return The interface. an excetion is thrown if a value field does not exist.
     */
    virtual epics::pvData::PVFieldPtr getValue() = 0;
    /**
     * @brief Return the interface to a scalar value field.
     * @return The interface for a scalar value field.
     * An exception is thown if no scalar value field.
     */
    virtual epics::pvData::PVScalarPtr getScalarValue() = 0;
    /**
     * @brief Return the interface to an array value field.
     * @return The interface.
     * An exception is thown if no array value field.
     */
    virtual std::tr1::shared_ptr<epics::pvData::PVArray> getArrayValue() = 0;
    /**
     * @brief Return the interface to a scalar array value field.
     * @return Return the interface.
     * An exception is thown if no scalar array value field.
     */
    virtual std::tr1::shared_ptr<epics::pvData::PVScalarArray> getScalarArrayValue() = 0;
    /**
     * @brief Get the boolean value. If value is not a boolean an exception is thrown
     * @return true or false.
     */
    virtual bool getBoolean() = 0;
    /**
     * @brief Get the value as a byte.
     * If value is not a byte scalar an exception is thrown.
     * @return The value.
     */
    virtual epics::pvData::int8 getByte() = 0;
    /**
     * @brief Get the value as a short.
     * If value is not a numeric scalar an exception is thrown
     * @return The value.
     * An exception is also thrown if the actualy type can cause an overflow.
     */
    virtual epics::pvData::int16 getShort() = 0;
    /**
     * @brief Get the value as an int.
     * If value is not a numeric scalar an exception is thrown.
     * An exception is also thrown if the actualy type can cause an overflow.
     */
    virtual epics::pvData::int32 getInt() = 0;
    /**
     * @brief Get the value as a long.
     * If value is not a numeric scalar an exception is thrown.
     * An exception is also thrown if the actualy type can cause an overflow.
     */
    virtual epics::pvData::int64 getLong() = 0;
    /**
     * @brief Get the value as an unsigned byte.
     * If value is not a numeric scalar an exception is thrown.
     * An exception is also thrown if the actualy type can cause an overflow.
     */
    virtual epics::pvData::uint8 getUByte() = 0;
    /**
     * @brief Get the value as a short.
     * If value is not a numeric scalar an exception is thrown.
     * An exception is also thrown if the actualy type can cause an overflow.
     */
    virtual epics::pvData::uint16 getUShort() = 0;
    /**
     * @brief Get the value as an unsigned int.
     * @return  If value is not a numeric scalar an exception is thrown
     */
    virtual epics::pvData::uint32 getUInt() = 0;
    /**
     * @brief Get the value as an unsigned long.
     * If value is not a numeric scalar an exception is thrown.
     * An exception is also thrown if the actualy type can cause an overflow.
     */
    virtual epics::pvData::uint64 getULong() = 0;
    /**
     * @brief Get the value as a float.
     * If value is not a numeric scalar an exception is thrown.
     * @return The value.
     */
    virtual float getFloat() = 0;
    /**
     * @brief Get the value as a double.
     * If value is not a numeric scalar an exception is thrown.
     * @return The value.
     */
    virtual double getDouble() = 0;
    /**
     * @brief Get the value as a string.
     * If value is not a string an exception is thrown
     * @return The value.
     */
    virtual std::string getString() = 0;
    
    /**
     * @brief Get the value as a boolean array.
     * If the value is not a boolean array an exception is thrown
     * @return The value.
     */
    virtual epics::pvData::shared_vector<epics::pvData::boolean>  getBooleanArray() = 0;
    /**
     * @brief Get the value as a byte array.
     * @return If the value is not a numeric array an exception is thrown
     */
    virtual epics::pvData::shared_vector<epics::pvData::int8>  getByteArray() = 0;
    /**
     * @brief Get the value as a short array.
     * If the value is not a numeric array an exception is thrown.
     * An exception is also thrown if the actualy type can cause an overflow.
     * @return The value.
     */
    virtual epics::pvData::shared_vector<epics::pvData::int16>  getShortArray() = 0;
    /**
     * @brief Get the value as an int array.
     * If the value is not a numeric array an exception is thrown.
     * An exception is also thrown if the actualy type can cause an overflow.
     * @return The value.
     */
    virtual epics::pvData::shared_vector<epics::pvData::int32>  getIntArray() = 0;
    /**
     * @brief Get the value as a long array.
     * If the value is not a numeric array an exception is thrown.
     * An exception is also thrown if the actualy type can cause an overflow.
     * @return The value.
     */
    virtual epics::pvData::shared_vector<epics::pvData::int64>  getLongArray() = 0;
    /**
     * @brief Get the value as an unsigned byte array.
     * If the value is not a numeric array an exception is thrown.
     * An exception is also thrown if the actualy type can cause an overflow.
     * @return The value.
     */
    virtual epics::pvData::shared_vector<epics::pvData::uint8>  getUByteArray() = 0;
    /**
     * @brief Get the value as an unsigned short array.
     * If the value is not a numeric array an exception is thrown.
     * An exception is also thrown if the actualy type can cause an overflow.
     * @return The value.
     */
    virtual epics::pvData::shared_vector<epics::pvData::uint16>  getUShortArray() = 0;
    /**
     * Get the value as an unsigned int array.
     * If the value is not a numeric array an exception is thrown.
     * An exception is also thrown if the actualy type can cause an overflow.
     * @return The value.
     */
    virtual epics::pvData::shared_vector<epics::pvData::uint32>  getUIntArray() = 0;
    /**
     * @brief Get the value as an unsigned long array.
     * If the value is not a numeric array an exception is thrown.
     * An exception is also thrown if the actualy type can cause an overflow.
     * @return The value.
     */
    virtual epics::pvData::shared_vector<epics::pvData::uint64>  getULongArray() = 0;
    /**
     * @brief Get the value as a float array.
     * If the value is not a numeric array an exception is thrown.
     * @return The value.
     */
    virtual epics::pvData::shared_vector<float>  getFloatArray() = 0;
    /**
     * @brief Get the value as a double array.
     * If the value is not a numeric array an exception is thrown.
     * @return The value.
     */
    virtual epics::pvData::shared_vector<double>  getDoubleArray() = 0;
    /**
     * @brief Get the value as a string array.
     * If the value is not a string array an exception is thrown.
     * @return The value.
     */
    virtual epics::pvData::shared_vector<std::string>  getStringArray() = 0;
};

/**
 * @brief This is a factory for creating an EasyPVStructure
 *
 * @author mrk
 */
class epicsShareClass EasyPVStructureFactory
{
public:
    /**
     * @brief Create a EasyPVAStructure.
     * @return The interface to the EasyPVAStructure.
     */
    static EasyPVStructurePtr createEasyPVStructure();
};


/**
 * @brief An easy to use alternative to ChannelGet.
 *
 * @author mrk
 */
class epicsShareClass EasyGet :
     public EasyPVStructure
{
public:
    POINTER_DEFINITIONS(EasyGet);
    /**
     * @brief destructor
     */
    virtual ~EasyGet(){}
    /** 
     * @brief destroy an resources used.
     */
    virtual void destroy() = 0;
    /**
     * @brief call issueConnect and then waitConnect.
     * An exception is thrown if connect fails.
     */
    virtual void connect() = 0;
    /**
     * @brief create the channelGet connection to the channel.
     * This can only be called once.
     * An exception is thrown if connect fails.
     */
    virtual void issueConnect() = 0;
    /**
     * @brief wait until the channelGet connection to the channel is complete.
     * @return status;
     */
    virtual epics::pvData::Status waitConnect() = 0;
    /**
     * @brief Call issueGet and then waitGet.
     * An exception is thrown if get fails.
     */
    virtual void get() = 0;
    /**
     * @brief Issue a get and return immediately.
     */
    virtual void issueGet() = 0;
    /**
     * @brief Wait until get completes.
     * If failure getStatus can be called to get reason.
     * @return (false,true) means (failure,success)
     */
    virtual epics::pvData::Status waitGet() = 0;

    /**
     * @brief Get the bitSet for the top level structure.
     * @return The bitSet.
     */
    virtual epics::pvData::BitSetPtr getBitSet() = 0;
};

/**
 * @brief This is a factory for creating an EasyGet.
 *
 * @author mrk
 */
class epicsShareClass EasyGetFactory
{
public:
    /**
     * @brief Create a EasyPVAStructure.
     * @param &easyPVA Interface to EasyPVA
     * @param easyChannel Interface to EasyChannel
     * @param channel Interface to Channel
     * @param pvRequest The request structure.
     * @return The interface to the EasyPVAStructure.
     */
    static EasyGetPtr createEasyGet(
        EasyPVAPtr const &easyPVA,
        EasyChannelPtr const & easyChannel,
        epics::pvAccess::Channel::shared_pointer const & channel,
        epics::pvData::PVStructurePtr const &pvRequest
    );
};

}}

#endif  /* EASYPVA_H */

/** @page Overview Documentation
 *
 * <a href = "easyPVA.html">easyPVA.html</a>
 *
 */


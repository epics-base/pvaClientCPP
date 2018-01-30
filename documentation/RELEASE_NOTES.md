EPICS 7 release 4.7.1
=====================

API changes to PvaClientMonitor
-------------------------------

The create method that had arguments for stateChangeRequester and monitorRequester no longer exists.

API changes to PvaClientGet, ..., PvaClientMonitor
--------------------------------------------------

pvaClientGet, ..., pvaClientMonitor all implemented PvaClientChannelStateChangeRequester.
This was never called and has been removed.

Works with pvDataCPP-7.0 and pvAccessCPP-6.0 versions
-----------------------------------------------------

Will not work with older versions of these modules.

destroy methods removed
-----------------------

All the destroy methods are removed since implementation is RAII compliant.

API changes to PvaClientMonitor
-------------------------------

The second argument of method

    static PvaClientMonitorPtr create(
        PvaClientPtr const &pvaClient,
        epics::pvAccess::Channel::shared_pointer const & channel,
        epics::pvData::PVStructurePtr const &pvRequest
    );

Is now changed to

    static PvaClientMonitorPtr create(
        PvaClientPtr const &pvaClient,
        PvaClientChannelPtr const & pvaClientChannel,
        epics::pvData::PVStructurePtr const &pvRequest
    );

A new method is also implemented

    static PvaClientMonitorPtr create(
        PvaClientPtr const &pvaClient,
        std::string const & channelName,
        std::string const & providerName,
        std::string const & request,
        PvaClientChannelStateChangeRequesterPtr const & stateChangeRequester,
        PvaClientMonitorRequesterPtr const & monitorRequester
    );


pvaClientCPP Version 4.2
========================

* The examples are moved to exampleCPP.
* Support for channelRPC is now available.
* In PvaClientMultiChannel checkConnected() now throws an exception if connect fails.



pvaClientCPP Version 4.1
========================


pvaClient is a synchronous API for pvAccess.


This is the first release of pvaClientCPP.
It provides an API that is similar to pvaClientJava.


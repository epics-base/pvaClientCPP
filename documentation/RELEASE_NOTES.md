EPICS V4 release 4.7
====================

Works with release/7.0 of pvDataCPP and release/6.0 of pvAccessCPP
------------------------------------------------------------------

Will not work with older versions.

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


EPICS V4 release 4.6
====================

* The examples are moved to exampleCPP.
* Support for channelRPC is now available.
* In PvaClientMultiChannel checkConnected() now throws an exception if connect fails.



EPICS V4 release 4.5
====================


pvaClient is a synchronous API for pvAccess.


This is the first release of pvaClientCPP.
It provides an API that is similar to pvaClientJava.


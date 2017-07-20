pvaClientCPP
============

pvaClient is a synchronous client interface to pvAccess,
which is callback based.
pvaClient is thus easier to use than pvAccess itself.

See documentation/pvaClientCPP.html for details.

Building
--------

If a proper RELEASE.local file exists one directory level above pvaClientCPP
then just type:

    make

It can also be built by:

    cp configure/ExampleRELEASE.local configure/RELEASE.local
    edit configure/RELEASE.local
    make


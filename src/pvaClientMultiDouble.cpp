/* pvaClientMultiDouble.cpp */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvData is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
/**
 * @author mrk
 * @date 2015.03
 */

#define epicsExportSharedSymbols
#include <pv/pvaClientMultiDouble.h>

using std::tr1::static_pointer_cast;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace std;

namespace epics { namespace pvaClient { 

PvaClientMultiDoublePtr PvaClientMultiDouble::create(
    PvaClientPtr const & pvaClient,
    PVStringArrayPtr const & channelName,
    double timeout,
    std::string const & providerName)
{
    PvaClientMultiChannelPtr pvaClientMultiChannel(
        PvaClientMultiChannel::create(pvaClient,channelName,providerName));
    Status status = pvaClientMultiChannel->connect(timeout,0);
    if(!status.isOK()) throw std::runtime_error(status.getMessage());
    return PvaClientMultiDoublePtr(new PvaClientMultiDouble(pvaClientMultiChannel));
}

PvaClientMultiDouble::PvaClientMultiDouble(PvaClientMultiChannelPtr const &pvaClientMultiChannel)
:
   pvaClientMultiChannel(pvaClientMultiChannel)
{}

PvaClientMultiDouble::~PvaClientMultiDouble()
{
}

void PvaClientMultiDouble::createGet()
{
    PvaClientChannelArrayPtr pvaClientChannelArray = pvaClientMultiChannel->getPvaClientChannelArray().lock();
    if(!pvaClientChannelArray)  throw std::runtime_error("pvaClientChannelArray is gone");
    shared_vector<const PvaClientChannelPtr> pvaClientChannels = *pvaClientChannelArray;
    size_t numChannel = pvaClientChannels.size();
    pvaClientGet = std::vector<PvaClientGetPtr>(numChannel,PvaClientGetPtr());
    bool allOK = true;
    string message;
    for(size_t i=0; i<numChannel; ++i)
    {
        pvaClientGet[i] = pvaClientChannels[i]->createGet("value");
        pvaClientGet[i]->issueConnect();
    }
    for(size_t i=0; i<numChannel; ++i)
    {
         Status status = pvaClientGet[i]->waitConnect();
         if(!status.isOK()) {
             message = "connect status " + status.getMessage();
             allOK = false;
             break;
         }
    }
    if(!allOK) throw std::runtime_error(message);
}

void PvaClientMultiDouble::createPut()
{
    PvaClientChannelArrayPtr pvaClientChannelArray = pvaClientMultiChannel->getPvaClientChannelArray().lock();
    if(!pvaClientChannelArray)  throw std::runtime_error("pvaClientChannelArray is gone");
    shared_vector<const PvaClientChannelPtr> pvaClientChannels = *pvaClientChannelArray;
    size_t numChannel = pvaClientChannels.size();
    pvaClientPut = std::vector<PvaClientPutPtr>(numChannel,PvaClientPutPtr());
    bool allOK = true;
    string message;
    for(size_t i=0; i<numChannel; ++i)
    {
        pvaClientPut[i] = pvaClientChannels[i]->createPut("value");
        pvaClientPut[i]->issueConnect();
    }
    for(size_t i=0; i<numChannel; ++i)
    {
         Status status = pvaClientPut[i]->waitConnect();
         if(!status.isOK()) {
             message = "connect status " + status.getMessage();
             allOK = false;
             break;
         }
    }
    if(!allOK) throw std::runtime_error(message);
}

epics::pvData::shared_vector<double> PvaClientMultiDouble::get()
{
    if(pvaClientGet.empty()) createGet();
    shared_vector<const string> channelNames = pvaClientMultiChannel->getChannelNames()->view();
    size_t numChannel = channelNames.size();
    epics::pvData::shared_vector<double> data(channelNames.size());
    for(size_t i=0; i<numChannel; ++i)
    {
        pvaClientGet[i]->issueGet();
    }
    for(size_t i=0; i<numChannel; ++i)
    {
        Status status = pvaClientGet[i]->waitGet();
        if(!status.isOK()) {
            string message = channelNames[i] + " " + status.getMessage();
            throw std::runtime_error(message);
        }
        data[i] = pvaClientGet[i]->getData()->getDouble();
    }
    return data;
}

void PvaClientMultiDouble::put(shared_vector<double> const &value)
{
    if(pvaClientPut.empty()) createPut();
    shared_vector<const string> channelNames = pvaClientMultiChannel->getChannelNames()->view();
    size_t numChannel = channelNames.size();
    for(size_t i=0; i<numChannel; ++i)
    {
        pvaClientPut[i]->getData()->putDouble(value[i]);
        pvaClientPut[i]->issuePut();
    }
    for(size_t i=0; i<numChannel; ++i)
    {
        Status status = pvaClientPut[i]->waitPut();
        if(!status.isOK()) {
            string message = channelNames[i] + " " + status.getMessage();
            throw std::runtime_error(message);
        }
    }
}


}}

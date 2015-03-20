/* easyMultiDouble.cpp */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvData is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
/**
 * @author mrk
 * @date 2015.03
 */

#include <pv/easyMultiDouble.h>

using std::tr1::static_pointer_cast;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace std;

namespace epics { namespace easyPVA { 

EasyMultiDoublePtr EasyMultiDouble::create(
    EasyPVAPtr const & easyPVA,
    PVStringArrayPtr const & channelName,
    double timeout,
    std::string const & providerName)
{
    EasyMultiChannelPtr easyMultiChannel(
        EasyMultiChannel::create(easyPVA,channelName,providerName));
    Status status = easyMultiChannel->connect(timeout,0);
    if(!status.isOK()) throw std::runtime_error(status.getMessage());
    return EasyMultiDoublePtr(new EasyMultiDouble(easyMultiChannel));
}

EasyMultiDouble::EasyMultiDouble(EasyMultiChannelPtr const &easyMultiChannel)
:
   easyMultiChannel(easyMultiChannel)
{}

EasyMultiDouble::~EasyMultiDouble()
{
}

void EasyMultiDouble::createGet()
{
    EasyChannelArrayPtr easyChannelArray = easyMultiChannel->getEasyChannelArray().lock();
    if(!easyChannelArray)  throw std::runtime_error("easyChannelArray is gone");
    shared_vector<const EasyChannelPtr> easyChannels = *easyChannelArray;
    size_t numChannel = easyChannels.size();
    easyGet = std::vector<EasyGetPtr>(numChannel,EasyGetPtr());
    bool allOK = true;
    string message;
    for(size_t i=0; i<numChannel; ++i)
    {
        easyGet[i] = easyChannels[i]->createGet("value");
        easyGet[i]->issueConnect();
    }
    for(size_t i=0; i<numChannel; ++i)
    {
         Status status = easyGet[i]->waitConnect();
         if(!status.isOK()) {
             message = "connect status " + status.getMessage();
             allOK = false;
             break;
         }
    }
    if(!allOK) throw std::runtime_error(message);
}

void EasyMultiDouble::createPut()
{
    EasyChannelArrayPtr easyChannelArray = easyMultiChannel->getEasyChannelArray().lock();
    if(!easyChannelArray)  throw std::runtime_error("easyChannelArray is gone");
    shared_vector<const EasyChannelPtr> easyChannels = *easyChannelArray;
    size_t numChannel = easyChannels.size();
    easyPut = std::vector<EasyPutPtr>(numChannel,EasyPutPtr());
    bool allOK = true;
    string message;
    for(size_t i=0; i<numChannel; ++i)
    {
        easyPut[i] = easyChannels[i]->createPut("value");
        easyPut[i]->issueConnect();
    }
    for(size_t i=0; i<numChannel; ++i)
    {
         Status status = easyPut[i]->waitConnect();
         if(!status.isOK()) {
             message = "connect status " + status.getMessage();
             allOK = false;
             break;
         }
    }
    if(!allOK) throw std::runtime_error(message);
}

epics::pvData::shared_vector<double> EasyMultiDouble::get()
{
    if(easyGet.empty()) createGet();
    shared_vector<const string> channelNames = easyMultiChannel->getChannelNames()->view();
    size_t numChannel = channelNames.size();
    epics::pvData::shared_vector<double> data(channelNames.size());
    for(size_t i=0; i<numChannel; ++i)
    {
        easyGet[i]->issueGet();
    }
    for(size_t i=0; i<numChannel; ++i)
    {
        Status status = easyGet[i]->waitGet();
        if(!status.isOK()) {
            string message = channelNames[i] + " " + status.getMessage();
            throw std::runtime_error(message);
        }
        data[i] = easyGet[i]->getData()->getDouble();
    }
    return data;
}

void EasyMultiDouble::put(shared_vector<double> const &value)
{
    if(easyPut.empty()) createPut();
    shared_vector<const string> channelNames = easyMultiChannel->getChannelNames()->view();
    size_t numChannel = channelNames.size();
    for(size_t i=0; i<numChannel; ++i)
    {
        easyPut[i]->getData()->putDouble(value[i]);
        easyPut[i]->issuePut();
    }
    for(size_t i=0; i<numChannel; ++i)
    {
        Status status = easyPut[i]->waitPut();
        if(!status.isOK()) {
            string message = channelNames[i] + " " + status.getMessage();
            throw std::runtime_error(message);
        }
    }
}


}}

/* pvaClientPutData.cpp */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvData is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
/**
 * @author mrk
 * @date 2015.02
 */

#include <typeinfo>
#include <sstream>

#include <pv/createRequest.h>
#include <pv/convert.h>

#define epicsExportSharedSymbols

#include <pv/pvaClient.h>

using std::tr1::static_pointer_cast;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace std;

namespace epics { namespace pvaClient {
static ConvertPtr convert = getConvert();
static string notCompatibleScalar("value is not a compatible scalar");
static string notDoubleArray("value is not a doubleArray");
static string notStringArray("value is not a stringArray");

class PvaClientPostHandlerPvt: public PostHandler
{
    PvaClientPutData * putData;
    size_t fieldNumber;
public:
    PvaClientPostHandlerPvt(PvaClientPutData *putData,size_t fieldNumber)
    : putData(putData),fieldNumber(fieldNumber){}
    void postPut() { putData->postPut(fieldNumber);}
};


PvaClientPutDataPtr PvaClientPutData::create(StructureConstPtr const & structure)
{
    if(PvaClient::getDebug()) cout << "PvaClientPutData::create\n";
    PvaClientPutDataPtr epv(new PvaClientPutData(structure));
    return epv;
}

PvaClientPutData::PvaClientPutData(StructureConstPtr const & structure)
: PvaClientData(structure)
{
    if(PvaClient::getDebug()) cout << "PvaClientPutData::PvaClientPutData\n";
    PVStructurePtr pvStructure(getPVDataCreate()->createPVStructure(structure));
    BitSetPtr bitSet(BitSetPtr(new BitSet(pvStructure->getNumberFields())));
    setData(pvStructure,bitSet);
    size_t nfields = pvStructure->getNumberFields();
    postHandler.resize(nfields);
    PVFieldPtr pvField;
    for(size_t i =0; i<nfields; ++i)
    {
        postHandler[i] = PostHandlerPtr(new PvaClientPostHandlerPvt(this, i));
        if(i==0) {
            pvField = pvStructure;
        } else {
            pvField = pvStructure->getSubField(i);
        }
        pvField->setPostHandler(postHandler[i]);
    }
}


void PvaClientPutData::putDouble(double value)
{
    if(PvaClient::getDebug()) cout << "PvaClientPutData::putDouble\n";
    PVScalarPtr pvScalar = getScalarValue();
    ScalarType scalarType = pvScalar->getScalar()->getScalarType();
    if(scalarType==pvDouble) {
        PVDoublePtr pvDouble = static_pointer_cast<PVDouble>(pvScalar);
         pvDouble->put(value);
         return;
    }
    if(!ScalarTypeFunc::isNumeric(scalarType)) {
        throw std::runtime_error(messagePrefix + notCompatibleScalar);
    }
    convert->fromDouble(pvScalar,value);
}

void PvaClientPutData::putString(std::string const & value)
{
    if(PvaClient::getDebug()) cout << "PvaClientPutData::putString\n";
    PVScalarPtr pvScalar = getScalarValue();
    convert->fromString(pvScalar,value);
}

void PvaClientPutData::putDoubleArray(shared_vector<const double> const & value)
{
    if(PvaClient::getDebug()) cout << "PvaClientPutData::putDoubleArray\n";
    PVScalarArrayPtr pvScalarArray = getScalarArrayValue();
    if(pvScalarArray->getScalarArray()->getElementType()!=pvDouble) {
        throw std::runtime_error(messagePrefix + notDoubleArray);
    }
    PVDoubleArrayPtr pv = static_pointer_cast<PVDoubleArray>(pvScalarArray);
    pv->replace(value);
}

void PvaClientPutData::putStringArray(shared_vector<const std::string> const & value)
{
    if(PvaClient::getDebug()) cout << "PvaClientPutData::putStringArray\n";
    PVScalarArrayPtr pvScalarArray = getScalarArrayValue();
    if(pvScalarArray->getScalarArray()->getElementType()!=pvString) {
        throw std::runtime_error(messagePrefix + notStringArray);
    }
    PVStringArrayPtr pv = static_pointer_cast<PVStringArray>(pvScalarArray);
    pv->replace(value);
}

void PvaClientPutData::putStringArray(std::vector<std::string> const & value)
{
    if(PvaClient::getDebug()) cout << "PvaClientPutData::putStringArray\n";
    PVScalarArrayPtr pvScalarArray = getScalarArrayValue();
    convert->fromStringArray(pvScalarArray,0,value.size(),value,0);
}

void PvaClientPutData::postPut(size_t fieldNumber)
{
    if(PvaClient::getDebug()) cout << "PvaClientPutData::postPut\n";
    getChangedBitSet()->set(fieldNumber);
}


}}

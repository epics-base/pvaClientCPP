/*exampleEasyNTMultiChannel.cpp */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvData is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
/**
 * @author mrk
 */

/* Author: Marty Kraimer */

#include <iostream>

#include <pv/easyNTMultiChannel.h>

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::easyPVA;
using namespace epics::nt;


static void example(EasyPVAPtr const &easyPVA)
{
    cout << "example ntMultiChannel\n";
    size_t num = 5;
    shared_vector<string> channelNames(num);
    channelNames[0] = "exampleDouble";
    channelNames[1] = "exampleDoubleArray";
    channelNames[2] = "exampleString";
    channelNames[3] = "exampleBoolean";
    channelNames[4] = "exampleEnum";
    PVStringArrayPtr pvNames =
        getPVDataCreate()->createPVScalarArray<PVStringArray>();
    pvNames->replace(freeze(channelNames));
    NTMultiChannelBuilderPtr builder = NTMultiChannel::createBuilder();
        StructureConstPtr structure = builder->
            addTimeStamp()->
            addSeverity() ->
            addStatus() ->
            addMessage() ->
            addSecondsPastEpoch() ->
            addNanoseconds() ->
            addUserTag() ->
            createStructure();
    EasyNTMultiChannelPtr easy = EasyNTMultiChannel::create(
       easyPVA,pvNames,structure);
    try {
        NTMultiChannelPtr nt = easy->get();
        cout << "initial\n" << nt->getPVStructure() << endl;

    } catch (std::runtime_error e) {
        cout << "exception " << e.what() << endl;
    }

}

static void exampleCA(EasyPVAPtr const &easyPVA)
{
    cout << "example ntMultiChannel\n";
    size_t num = 5;
    shared_vector<string> channelNames(num);
    channelNames[0] = "double00";
    channelNames[1] = "doubleArray";
    channelNames[2] = "string00";
    channelNames[3] = "mbbiwierd";
    channelNames[4] = "enum01";
    PVStringArrayPtr pvNames =
        getPVDataCreate()->createPVScalarArray<PVStringArray>();
    pvNames->replace(freeze(channelNames));
    NTMultiChannelBuilderPtr builder = NTMultiChannel::createBuilder();
        StructureConstPtr structure = builder->
            addTimeStamp()->
            addSeverity() ->
            addStatus() ->
            addMessage() ->
            addSecondsPastEpoch() ->
            addNanoseconds() ->
            addUserTag() ->
            createStructure();
    EasyNTMultiChannelPtr easy = EasyNTMultiChannel::create(
       easyPVA,pvNames,structure,5.0,"ca");
    try {
        NTMultiChannelPtr nt = easy->get();
        cout << "initial\n" << nt->getPVStructure() << endl;

    } catch (std::runtime_error e) {
        cout << "exception " << e.what() << endl;
    }

}


int main(int argc,char *argv[])
{
    EasyPVAPtr easyPVA = EasyPVA::create();
    example(easyPVA);
    exampleCA(easyPVA);
    return 0;
}

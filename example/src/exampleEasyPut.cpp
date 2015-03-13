/*exampleEasyProcess.cpp */
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

#include <pv/easyPVA.h>

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::easyPVA;


static void examplePut(EasyPVAPtr const &easyPVA)
{
    cout << "example process\n";
    EasyChannelPtr channel = easyPVA->channel("exampleDouble");
    EasyPutPtr put = channel->put();
    EasyPutDataPtr putData = put->getData();
    try {
        putData->putDouble(3.0); put->put();
        cout <<  channel->get("field()")->getData()->showChanged(cout) << endl;
        putData->putDouble(4.0); put->put();
        cout <<  channel->get("field()")->getData()->showChanged(cout) << endl;
    } catch (std::runtime_error e) {
        cout << "exception " << e.what() << endl;
    }
}


int main(int argc,char *argv[])
{
    EasyPVAPtr easyPVA = EasyPVA::create();
    examplePut(easyPVA);
    return 0;
}

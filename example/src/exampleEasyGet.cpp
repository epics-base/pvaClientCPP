/*exampleEasyGet.cpp */
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
#include <pv/clientFactory.h>

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::easyPVA;

int main(int argc,char *argv[])
{
    ClientFactory::start();
    EasyPVAPtr easyPVA = EasyPVA::create();

    double value;
    try {
        cout << "short way\n";
        value =  easyPVA->createChannel("exampleDouble")->createGet()->getDouble();
        cout << "as double " << value << endl;
    } catch (std::runtime_error e) {
        cout << "exception " << e.what() << endl;
    }
    try {
        cout << "long way\n";
        EasyChannelPtr easyChannel = easyPVA->createChannel("exampleDouble");
        easyChannel->connect(2.0);
        EasyGetPtr easyGet = easyChannel->createGet();
        value = easyGet->getDouble();
        cout << "as double " << value << endl;
    } catch (std::runtime_error e) {
        cout << "exception " << e.what() << endl;
    }
    cout << "done\n";
    ClientFactory::stop();
    return 0;
}

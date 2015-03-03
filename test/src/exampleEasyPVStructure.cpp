/*exampleEasyPVStructure.cpp */
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

int main(int argc,char *argv[])
{
    EasyPVAPtr easyPVA = EasyPVA::create();

    StandardPVFieldPtr standardPVField = getStandardPVField();
    EasyPVStructurePtr easyStruct = easyPVA->createEasyPVStructure();
    PVStructurePtr pvStructure = standardPVField->scalar(pvDouble,"alarm,timeStamp");
    easyStruct->setPVStructure(pvStructure);
    cout << easyStruct->getPVStructure() << endl;
    cout << "as double " << easyStruct->getDouble() << endl;
    cout << "as float " << easyStruct->getFloat() << endl;
    try {
        cout << "as ubyte " << easyStruct->getUByte() << endl;
    } catch (std::runtime_error e) {
        cout << "exception " << e.what() << endl;
    }
    pvStructure = standardPVField->scalar(pvInt,"alarm,timeStamp");
    easyStruct->setPVStructure(pvStructure);
    cout << easyStruct->getPVStructure() << endl;
    cout << "as double " << easyStruct->getDouble() << endl;
    cout << "as float " << easyStruct->getFloat() << endl;
    cout << "as int " << easyStruct->getInt() << endl;
    cout << "as uint " << easyStruct->getUInt() << endl;
    cout << "as long " << easyStruct->getLong() << endl;
    cout << "as ulong " << easyStruct->getULong() << endl;
    try {
        cout << "as byte " << easyStruct->getByte() << endl;
    } catch (std::runtime_error e) {
        cout << "exception " << e.what() << endl;
    }
    try {
        cout << "as ubyte " << easyStruct->getUByte() << endl;
    } catch (std::runtime_error e) {
        cout << "exception " << e.what() << endl;
    }
    try {
        cout << "as short " << easyStruct->getShort() << endl;
    } catch (std::runtime_error e) {
        cout << "exception " << e.what() << endl;
    }
    try {
        cout << "as ushort " << easyStruct->getUShort() << endl;
    } catch (std::runtime_error e) {
        cout << "exception " << e.what() << endl;
    }

    return 0;
}

#include <QCoreApplication>
#include "AbstractNetworkInterface.h"
#include "pe.h"

int main (int argc, char *argv[]) {
    QCoreApplication carterMessage(argc, argv);
    NetworkImplementation ANI;
    ANI.initialise("127.0.0.1", 3000);
    ANI.close();
    return carterMessage.exec();
}

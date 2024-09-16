#include <QCoreApplication>
#include "AbstractNetworkInterface.h"

int main (int argc, char *argv[]) {
    QCoreApplication carterMessage(argc, argv);
    NetworkImplementation ANI;
    return carterMessage.exec();
}

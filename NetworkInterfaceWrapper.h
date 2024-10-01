#ifndef NETWORKINTERFACEWRAPPER_H
#define NETWORKINTERFACEWRAPPER_H

#include <QObject>
#include <QString>
#include <QVariant>
#include "AbstractNetworkInterface.h"

class NetworkInterfaceWrapper : public QObject
{
    Q_OBJECT

public:
    explicit NetworkInterfaceWrapper(AbstractNetworkInterface* interface, QObject *parent = nullptr);

public slots:
    void initialise(const QString& address, unsigned short port);
    bool sendPE(const QVariantMap& pe);
    bool sendEmitter(const QVariantMap& emitter);
    bool sendBlob(const QString& blobString);
    bool sendComplexBlob(const QVariantMap& pe, const QVariantMap& emitter, const QVariantMap& doubleMap);
    bool sendPESetting(const QString& setting, const QString& id, int updateVal);
    bool sendEmitterSetting(const QString& setting, const QString& id, int updateVal);
    QVariantList receiveSetting();
    QVariantMap receivePE();
    QVariantMap receiveEmitter();
    QVariantList receiveBlob();
    QVariantList receiveComplexBlob();
    void close();

signals:
    void error(const QString& message);

private:
    AbstractNetworkInterface* m_interface;

    PE convertToPE(const QVariantMap& map);
    Emitter convertToEmitter(const QVariantMap& map);
    QVariantMap convertFromPE(const PE& pe);
    QVariantMap convertFromEmitter(const Emitter& emitter);
    std::map<std::string, double> convertToDoubleMap(const QVariantMap& map);
};

#endif // NETWORKINTERFACEWRAPPER_H

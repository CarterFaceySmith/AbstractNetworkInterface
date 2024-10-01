#include "NetworkInterfaceWrapper.h"
#include <QJsonObject>
#include <QJsonArray>

/*!
    \class NetworkInterfaceWrapper
    \brief A Qt wrapper for AbstractNetworkInterface.

    This class provides a Qt-friendly interface to AbstractNetworkInterface,
    allowing easy integration with Qt applications. It handles conversions
    between Qt and standard C++ types, and provides error handling through
    Qt's signal-slot mechanism.
*/

NetworkInterfaceWrapper::NetworkInterfaceWrapper(AbstractNetworkInterface* interface, QObject *parent)
    : QObject(parent), m_interface(interface)
{
}

/*!
    \fn void NetworkInterfaceWrapper::initialise(const QString& address, unsigned short port)
    \brief Initializes the network interface.
    \param address The IP address to connect to.
    \param port The port number to use.

    This function attempts to initialize the underlying network interface.
    If initialization fails, it emits an error signal with a description.
*/
void NetworkInterfaceWrapper::initialise(const QString& address, unsigned short port)
{
    try {
        m_interface->initialise(address.toStdString(), port);
    } catch (const std::exception& e) {
        emit error(QString("Failed to initialize: %1").arg(e.what()));
    }
}

/*!
    \fn bool NetworkInterfaceWrapper::sendPE(const QVariantMap& pe)
    \brief Sends a Platform Element (PE) over the network.
    \param pe A QVariantMap representing the PE to send.
    \return True if the PE was sent successfully, false otherwise.

    This function converts the provided QVariantMap to a PE object and sends it.
    If an error occurs, it emits an error signal with a description.
*/
bool NetworkInterfaceWrapper::sendPE(const QVariantMap& pe)
{
    try {
        return m_interface->sendPE(convertToPE(pe));
    } catch (const std::exception& e) {
        emit error(QString("Failed to send PE: %1").arg(e.what()));
        return false;
    }
}

/*!
    \fn bool NetworkInterfaceWrapper::sendEmitter(const QVariantMap& emitter)
    \brief Sends an Emitter over the network.
    \param emitter A QVariantMap representing the Emitter to send.
    \return True if the Emitter was sent successfully, false otherwise.

    This function converts the provided QVariantMap to an Emitter object and sends it.
    If an error occurs, it emits an error signal with a description.
*/
bool NetworkInterfaceWrapper::sendEmitter(const QVariantMap& emitter)
{
    try {
        return m_interface->sendEmitter(convertToEmitter(emitter));
    } catch (const std::exception& e) {
        emit error(QString("Failed to send Emitter: %1").arg(e.what()));
        return false;
    }
}

/*!
    \fn bool NetworkInterfaceWrapper::sendBlob(const QString& blobString)
    \brief Sends a blob string over the network.
    \param blobString The blob string to send.
    \return True if the blob was sent successfully, false otherwise.

    This function sends the provided blob string.
    If an error occurs, it emits an error signal with a description.
*/
bool NetworkInterfaceWrapper::sendBlob(const QString& blobString)
{
    try {
        return m_interface->sendBlob(blobString.toStdString());
    } catch (const std::exception& e) {
        emit error(QString("Failed to send blob: %1").arg(e.what()));
        return false;
    }
}

/*!
    \fn bool NetworkInterfaceWrapper::sendComplexBlob(const QVariantMap& pe, const QVariantMap& emitter, const QVariantMap& doubleMap)
    \brief Sends a complex blob over the network.
    \param pe A QVariantMap representing the PE part of the complex blob.
    \param emitter A QVariantMap representing the Emitter part of the complex blob.
    \param doubleMap A QVariantMap representing additional double values.
    \return True if the complex blob was sent successfully, false otherwise.

    This function converts the provided QVariantMaps to their respective types and sends them as a complex blob.
    If an error occurs, it emits an error signal with a description.
*/
bool NetworkInterfaceWrapper::sendComplexBlob(const QVariantMap& pe, const QVariantMap& emitter, const QVariantMap& doubleMap)
{
    try {
        return m_interface->sendComplexBlob(convertToPE(pe), convertToEmitter(emitter), convertToDoubleMap(doubleMap));
    } catch (const std::exception& e) {
        emit error(QString("Failed to send complex blob: %1").arg(e.what()));
        return false;
    }
}

/*!
    \fn bool NetworkInterfaceWrapper::sendPESetting(const QString& setting, const QString& id, int updateVal)
    \brief Sends a PE setting update.
    \param setting The setting to update.
    \param id The ID of the PE.
    \param updateVal The new value for the setting.
    \return True if the setting was sent successfully, false otherwise.

    This function sends an update for a specific PE setting.
    If an error occurs, it emits an error signal with a description.
*/
bool NetworkInterfaceWrapper::sendPESetting(const QString& setting, const QString& id, int updateVal)
{
    try {
        return m_interface->sendPESetting(setting.toStdString(), id.toStdString(), updateVal);
    } catch (const std::exception& e) {
        emit error(QString("Failed to send PE setting: %1").arg(e.what()));
        return false;
    }
}

/*!
    \fn bool NetworkInterfaceWrapper::sendEmitterSetting(const QString& setting, const QString& id, int updateVal)
    \brief Sends an Emitter setting update.
    \param setting The setting to update.
    \param id The ID of the Emitter.
    \param updateVal The new value for the setting.
    \return True if the setting was sent successfully, false otherwise.

    This function sends an update for a specific Emitter setting.
    If an error occurs, it emits an error signal with a description.
*/
bool NetworkInterfaceWrapper::sendEmitterSetting(const QString& setting, const QString& id, int updateVal)
{
    try {
        return m_interface->sendEmitterSetting(setting.toStdString(), id.toStdString(), updateVal);
    } catch (const std::exception& e) {
        emit error(QString("Failed to send Emitter setting: %1").arg(e.what()));
        return false;
    }
}

/*!
    \fn QVariantList NetworkInterfaceWrapper::receiveSetting()
    \brief Receives a setting update.
    \return A QVariantList containing the setting details, or an empty list if an error occurred.

    This function receives a setting update and returns it as a QVariantList.
    The list contains the type, ID, setting name, and value.
    If an error occurs, it emits an error signal with a description.
*/
QVariantList NetworkInterfaceWrapper::receiveSetting()
{
    try {
        auto [type, id, setting, value] = m_interface->receiveSetting();
        return QVariantList{QString::fromStdString(type), QString::fromStdString(id), QString::fromStdString(setting), value};
    } catch (const std::exception& e) {
        emit error(QString("Failed to receive setting: %1").arg(e.what()));
        return QVariantList();
    }
}

/*!
    \fn QVariantMap NetworkInterfaceWrapper::receivePE()
    \brief Receives a Platform Element (PE).
    \return A QVariantMap representing the received PE, or an empty map if an error occurred.

    This function receives a PE and converts it to a QVariantMap.
    If an error occurs, it emits an error signal with a description.
*/
QVariantMap NetworkInterfaceWrapper::receivePE()
{
    try {
        return convertFromPE(m_interface->receivePE());
    } catch (const std::exception& e) {
        emit error(QString("Failed to receive PE: %1").arg(e.what()));
        return QVariantMap();
    }
}

/*!
    \fn QVariantMap NetworkInterfaceWrapper::receiveEmitter()
    \brief Receives an Emitter.
    \return A QVariantMap representing the received Emitter, or an empty map if an error occurred.

    This function receives an Emitter and converts it to a QVariantMap.
    If an error occurs, it emits an error signal with a description.
*/
QVariantMap NetworkInterfaceWrapper::receiveEmitter()
{
    try {
        return convertFromEmitter(m_interface->receiveEmitter());
    } catch (const std::exception& e) {
        emit error(QString("Failed to receive Emitter: %1").arg(e.what()));
        return QVariantMap();
    }
}

/*!
    \fn QVariantList NetworkInterfaceWrapper::receiveBlob()
    \brief Receives a blob.
    \return A QVariantList containing the received blob strings, or an empty list if an error occurred.

    This function receives a blob and converts it to a QVariantList of strings.
    If an error occurs, it emits an error signal with a description.
*/
QVariantList NetworkInterfaceWrapper::receiveBlob()
{
    try {
        auto blob = m_interface->receiveBlob();
        QVariantList result;
        for (const auto& str : blob) {
            result.append(QString::fromStdString(str));
        }
        return result;
    } catch (const std::exception& e) {
        emit error(QString("Failed to receive blob: %1").arg(e.what()));
        return QVariantList();
    }
}

/*!
    \fn QVariantList NetworkInterfaceWrapper::receiveComplexBlob()
    \brief Receives a complex blob.
    \return A QVariantList containing the PE, Emitter, and double map components of the complex blob, or an empty list if an error occurred.

    This function receives a complex blob and converts its components to Qt-friendly types.
    If an error occurs, it emits an error signal with a description.
*/
QVariantList NetworkInterfaceWrapper::receiveComplexBlob()
{
    try {
        auto [pe, emitter, doubleMap] = m_interface->receiveComplexBlob();
        QVariantMap convertedDoubleMap;
        for (const auto& [key, value] : doubleMap) {
            convertedDoubleMap[QString::fromStdString(key)] = value;
        }
        return QVariantList{convertFromPE(pe), convertFromEmitter(emitter), convertedDoubleMap};
    } catch (const std::exception& e) {
        emit error(QString("Failed to receive complex blob: %1").arg(e.what()));
        return QVariantList();
    }
}

/*!
    \fn void NetworkInterfaceWrapper::close()
    \brief Closes the network connection.

    This function attempts to close the underlying network connection.
    If an error occurs, it emits an error signal with a description.
*/
void NetworkInterfaceWrapper::close()
{
    try {
        m_interface->close();
    } catch (const std::exception& e) {
        emit error(QString("Failed to close connection: %1").arg(e.what()));
    }
}

// Helper functions - private methods

PE NetworkInterfaceWrapper::convertToPE(const QVariantMap& map)
{
    PE pe(
        map["id"].toString(),
        map["type"].toString(),
        map["lat"].toDouble(),
        map["lon"].toDouble(),
        map["altitude"].toDouble(),
        map["speed"].toDouble(),
        map["apd"].toString(),
        map["priority"].toString(),
        map["jam"].toBool(),
        map["ghost"].toBool()
    );
    pe.heading = map["heading"].toDouble();
    pe.category = static_cast<PE::PECategory>(map["category"].toInt());
    pe.state = map["state"].toString();
    return pe;
}

Emitter NetworkInterfaceWrapper::convertToEmitter(const QVariantMap& map)
{
    Emitter emitter(
        map["id"].toString(),
        map["type"].toString(),
        map["category"].toString(),
        map["lat"].toDouble(),
        map["lon"].toDouble(),
        map["freqMin"].toDouble(),
        map["freqMax"].toDouble(),
        map["active"].toBool(),
        map["eaPriority"].toString(),
        map["esPriority"].toString(),
        map["jamResponsible"].toBool(),
        map["reactiveEligible"].toBool(),
        map["preemptiveEligible"].toBool(),
        map["consentRequired"].toBool(),
        map["jam"].toBool()
    );
    emitter.altitude = map["altitude"].toDouble();
    emitter.heading = map["heading"].toDouble();
    emitter.speed = map["speed"].toDouble();
    emitter.operatorManaged = map["operatorManaged"].toBool();
    emitter.jamIneffective = map["jamIneffective"].toInt();
    emitter.jamEffective = map["jamEffective"].toInt();
    return emitter;
}

QVariantMap NetworkInterfaceWrapper::convertFromPE(const PE& pe)
{
    return QVariantMap{
        {"id", pe.id},
        {"type", pe.type},
        {"lat", pe.lat},
        {"lon", pe.lon},
        {"altitude", pe.altitude},
        {"speed", pe.speed},
        {"heading", pe.heading},
        {"apd", pe.apd},
        {"priority", pe.priority},
        {"jam", pe.jam},
        {"ghost", pe.ghost},
        {"category", static_cast<int>(pe.category)},
        {"state", pe.state}
    };
}

QVariantMap NetworkInterfaceWrapper::convertFromEmitter(const Emitter& emitter)
{
    return QVariantMap{
        {"id", emitter.id},
        {"type", emitter.type},
        {"category", emitter.category},
        {"lat", emitter.lat},
        {"lon", emitter.lon},
        {"altitude", emitter.altitude},
        {"heading", emitter.heading},
        {"speed", emitter.speed},
        {"freqMin", emitter.freqMin},
        {"freqMax", emitter.freqMax},
        {"active", emitter.active},
        {"eaPriority", emitter.eaPriority},
        {"esPriority", emitter.esPriority},
        {"jamResponsible", emitter.jamResponsible},
        {"reactiveEligible", emitter.reactiveEligible},
        {"preemptiveEligible", emitter.preemptiveEligible},
        {"consentRequired", emitter.consentRequired},
        {"operatorManaged", emitter.operatorManaged},
        {"jam", emitter.jam},
        {"jamIneffective", emitter.jamIneffective},
        {"jamEffective", emitter.jamEffective}
    };
}

std::map<std::string, double> NetworkInterfaceWrapper::convertToDoubleMap(const QVariantMap& map)
{
    std::map<std::string, double> result;
    for (auto it = map.begin(); it != map.end(); ++it) {
        result[it.key().toStdString()] = it.value().toDouble();
    }
    return result;
}

#include "AbstractNetworkInterface.h"
#include "emitter.h"
#include "pe.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <iostream>
#include <sstream>
#include <stdexcept>

/*!
    \class NetworkImplementation
    \brief Implements network communication for PE and Emitter data transfer.

    This class provides methods for sending and receiving PE and Emitter data,
    as well as individual settings and complex blobs over a TCP network connection.
*/

/*!
    \fn NetworkImplementation::NetworkImplementation()
    \brief Constructs a NetworkImplementation object.

    Initializes the socket with a new boost::asio::ip::tcp::socket.
*/
NetworkImplementation::NetworkImplementation()
    : socket(std::make_unique<boost::asio::ip::tcp::socket>(io_context)) {}

/*!
    \fn void NetworkImplementation::initialise(const std::string& address, unsigned short port)
    \brief Initializes the network connection.
    \param address The IP address to connect to.
    \param port The port number to connect to.
*/
void NetworkImplementation::initialise(const std::string& address, unsigned short port) {
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(address), port);
    socket->connect(endpoint);
}

/*!
    \fn boost::asio::ip::tcp::socket* NetworkImplementation::getSocket()
    \brief Returns a pointer to the underlying socket.
    \return A pointer to the boost::asio::ip::tcp::socket.
*/
boost::asio::ip::tcp::socket* NetworkImplementation::getSocket() {
    return socket.get();
}

/*!
    \fn void NetworkImplementation::validateAndPrintDataBufferSize(std::string dataBuff, std::string funcName)
    \brief Validates and prints the size of the data buffer.
    \param dataBuff The data buffer to validate.
    \param funcName The name of the function calling this method.
*/
void NetworkImplementation::validateAndPrintDataBufferSize(std::string dataBuff, std::string funcName) {
    dataBuff.data() ? std::cout << funcName << " - Received intact data buffer of length: "
                                << dataBuff.length() << std::endl
                    : std::cerr << "Received invalid data buffer of length: "
                                << dataBuff.length() << std::endl;
}

/*!
    \fn bool NetworkImplementation::sendPESetting(const std::string& setting, const std::string& id, int updateVal)
    \brief Sends a PE setting update.
    \param setting The name of the setting to update.
    \param id The ID of the PE.
    \param updateVal The new value for the setting.
    \return True if the setting was sent successfully, false otherwise.
*/
bool NetworkImplementation::sendPESetting(const std::string& setting, const std::string& id, int updateVal) {
    QJsonObject json;
    json["type"] = "PE_SETTING";
    json["id"] = QString::fromStdString(id);
    json["setting"] = QString::fromStdString(setting);
    json["value"] = updateVal;

    QJsonDocument doc(json);
    std::string data = doc.toJson(QJsonDocument::Compact).toStdString() + "\n";

    std::cout << "SENDING PE SETTING: " << data;
    boost::asio::write(*socket, boost::asio::buffer(data));
    return true;
}

/*!
    \fn bool NetworkImplementation::sendEmitterSetting(const std::string& setting, const std::string& id, int updateVal)
    \brief Sends an Emitter setting update.
    \param setting The name of the setting to update.
    \param id The ID of the Emitter.
    \param updateVal The new value for the setting.
    \return True if the setting was sent successfully, false otherwise.
*/
bool NetworkImplementation::sendEmitterSetting(const std::string& setting, const std::string& id, int updateVal) {
    QJsonObject json;
    json["type"] = "EMITTER_SETTING";
    json["id"] = QString::fromStdString(id);
    json["setting"] = QString::fromStdString(setting);
    json["value"] = updateVal;

    QJsonDocument doc(json);
    std::string data = doc.toJson(QJsonDocument::Compact).toStdString() + "\n";

    std::cout << "SENDING EMITTER SETTING: " << data;
    boost::asio::write(*socket, boost::asio::buffer(data));
    return true;
}

/*!
    \fn std::tuple<std::string, std::string, std::string, int> NetworkImplementation::receiveSetting()
    \brief Receives a setting update.
    \return A tuple containing the type of setting, ID, setting name, and new value.
*/
std::tuple<std::string, std::string, std::string, int> NetworkImplementation::receiveSetting() {
    boost::asio::streambuf buf;
    boost::asio::read_until(*socket, buf, '\n');
    std::string data{boost::asio::buffers_begin(buf.data()),
                     boost::asio::buffers_end(buf.data())};
    buf.consume(buf.size());

    std::cout << "RECEIVED SETTING: " << data;

    QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(data).toUtf8());
    if (doc.isNull()) {
        throw std::runtime_error("Invalid JSON data for setting deserialization");
    }
    QJsonObject json = doc.object();

    std::string type = json["type"].toString().toStdString();
    std::string id = json["id"].toString().toStdString();
    std::string setting = json["setting"].toString().toStdString();
    int value = json["value"].toInt();

    return std::make_tuple(type, id, setting, value);
}

/*!
    \fn bool NetworkImplementation::sendBlob(const std::string& blobString)
    \brief Sends a blob of data.
    \param blobString The blob data to send.
    \return True if the blob was sent successfully, false otherwise.
*/
bool NetworkImplementation::sendBlob(const std::string& blobString) {
    boost::asio::write(*socket, boost::asio::buffer(blobString + "\n"));
    return true;
}

/*!
    \fn bool NetworkImplementation::sendPE(const PE& pe)
    \brief Sends a PE object.
    \param pe The PE object to send.
    \return True if the PE was sent successfully, false otherwise.
*/
bool NetworkImplementation::sendPE(const PE& pe) {
    // Basic data validation
    if (pe.id.length() < 3 || pe.altitude < 0) {
        std::cout << "Invalid data field in pe object..." << std::endl;
        return false;
    }
    std::string data = serializePE(pe) + "\n";  // Add newline to separate messages
    std::cout << "SENDING PE DATA: " << data;
    boost::asio::write(*socket, boost::asio::buffer(data));
    return true;
}

/*!
    \fn bool NetworkImplementation::sendEmitter(const Emitter& emitter)
    \brief Sends an Emitter object.
    \param emitter The Emitter object to send.
    \return True if the Emitter was sent successfully, false otherwise.
*/
bool NetworkImplementation::sendEmitter(const Emitter& emitter) {
    // Basic data validation
    if (emitter.id.length() < 3 || emitter.altitude < 0) {
        std::cout << "Invalid data field in emitter object..." << std::endl;
        return false;
    }
    std::string data = serializeEmitter(emitter);
    boost::asio::write(*socket, boost::asio::buffer(data));
    return true;
}

/*!
    \fn bool NetworkImplementation::sendComplexBlob(const PE& pe, const Emitter& emitter, const std::map<std::string, double>& doubleMap)
    \brief Sends a complex blob containing a PE, an Emitter, and a map of doubles.
    \param pe The PE object to include in the blob.
    \param emitter The Emitter object to include in the blob.
    \param doubleMap A map of string keys to double values to include in the blob.
    \return True if the complex blob was sent successfully, false otherwise.
*/
bool NetworkImplementation::sendComplexBlob(const PE& pe, const Emitter& emitter, const std::map<std::string, double>& doubleMap) {
    std::string data = serializeComplexBlob(pe, emitter, doubleMap);
    std::cout << "SENDING COMPLEX BLOB:\n" << data << std::endl;
    boost::asio::write(*socket, boost::asio::buffer(data));
    return true;
}

/*!
    \fn std::vector<PE> NetworkImplementation::receivePEs()
    \brief Receives a vector of PE objects.
    \return A vector of received PE objects.
*/
std::vector<PE> NetworkImplementation::receivePEs() {
    boost::asio::streambuf buf;
    boost::asio::read_until(*socket, buf, '\n');
    std::string data{boost::asio::buffers_begin(buf.data()),
                     boost::asio::buffers_end(buf.data())};
    buf.consume(buf.size());

    std::cout << "RECEIVED PES: " << data;
    std::vector<PE> pes;
    std::istringstream iss(data);
    std::string line;
    while (std::getline(iss, line)) {
        if (!line.empty()) {
            pes.push_back(deserializePE(line));
        }
    }
    return pes;
}

/*!
    \fn std::vector<Emitter> NetworkImplementation::receiveEmitters()
    \brief Receives a vector of Emitter objects.
    \return A vector of received Emitter objects.
*/
std::vector<Emitter> NetworkImplementation::receiveEmitters() {
    boost::asio::streambuf buf;
    boost::asio::read_until(*socket, buf, '\n');
    std::string data{boost::asio::buffers_begin(buf.data()),
                     boost::asio::buffers_end(buf.data())};
    validateAndPrintDataBufferSize(data, "receiveEmitters");

    return deserializeEmitters(data);
}

/*!
    \fn std::vector<std::string> NetworkImplementation::receiveBlob()
    \brief Receives a blob of data.
    \return A vector of strings containing the received blob data.
*/
std::vector<std::string> NetworkImplementation::receiveBlob() {
    boost::asio::streambuf buf;
    boost::asio::read_until(*socket, buf, '\n');
    std::string data{boost::asio::buffers_begin(buf.data()),
                     boost::asio::buffers_end(buf.data())};
    validateAndPrintDataBufferSize(data, "receiveBlob");

    std::vector<std::string> result;
    result.push_back(data);
    return result;
}

/*!
    \fn std::tuple<PE, Emitter, std::map<std::string, double>> NetworkImplementation::receiveComplexBlob()
    \brief Receives a complex blob containing a PE, an Emitter, and a map of doubles.
    \return A tuple containing the received PE, Emitter, and map of doubles.
*/
std::tuple<PE, Emitter, std::map<std::string, double>> NetworkImplementation::receiveComplexBlob() {
    boost::asio::streambuf buf;
    boost::asio::read_until(*socket, buf, '\n');
    std::string data{boost::asio::buffers_begin(buf.data()),
                     boost::asio::buffers_begin(buf.data()) + buf.size() - 1}; // Remove the trailing newline
    buf.consume(buf.size()); // Consume the read data
    std::cout << "RECEIVED COMPLEX BLOB:\n" << data << std::endl;
    validateAndPrintDataBufferSize(data, "receiveComplexBlob");
    return deserializeComplexBlob(data);
}

/*!
    \fn void NetworkImplementation::close()
    \brief Closes the network connection.
*/
void NetworkImplementation::close() {
    socket->close();
}

/*!
    \fn std::string NetworkImplementation::serializePE(const PE& pe)
    \brief Serializes a PE object to a JSON string.
    \param pe The PE object to serialize.
    \return A JSON string representation of the PE object.
*/
std::string NetworkImplementation::serializePE(const PE& pe) {
    QJsonObject json;
    json["id"] = QString::fromStdString(pe.id.toStdString());
    json["type"] = QString::fromStdString(pe.type.toStdString());
    json["lat"] = pe.lat;
    json["lon"] = pe.lon;
    json["altitude"] = pe.altitude;
    json["speed"] = pe.speed;
    json["heading"] = pe.heading;
    json["apd"] = QString::fromStdString(pe.apd.toStdString());
    json["priority"] = QString::fromStdString(pe.priority.toStdString());
    json["jam"] = pe.jam;
    json["ghost"] = pe.ghost;
    json["category"] = static_cast<int>(pe.category);
    json["state"] = QString::fromStdString(pe.state.toStdString());
    QJsonDocument doc(json);
    return doc.toJson(QJsonDocument::Compact).toStdString() + "\n";
}

/*!
    \fn std::string NetworkImplementation::serializeEmitter(const Emitter& emitter)
    \brief Serializes an Emitter object to a JSON string.
    \param emitter The Emitter object to serialize.
    \return A JSON string representation of the Emitter object.
*/
std::string NetworkImplementation::serializeEmitter(const Emitter& emitter) {
    QJsonObject json;
    json["id"] = QString::fromStdString(emitter.id.toStdString());
    json["type"] = QString::fromStdString(emitter.type.toStdString());
    json["category"] = QString::fromStdString(emitter.category.toStdString());
    json["lat"] = emitter.lat;
    json["lon"] = emitter.lon;
    json["altitude"] = emitter.altitude;
    json["heading"] = emitter.heading;
    json["speed"] = emitter.speed;
    json["freqMin"] = emitter.freqMin;
    json["freqMax"] = emitter.freqMax;
    json["active"] = emitter.active;
    json["eaPriority"] = QString::fromStdString(emitter.eaPriority.toStdString());
    json["esPriority"] = QString::fromStdString(emitter.esPriority.toStdString());
    json["jamResponsible"] = emitter.jamResponsible;
    json["reactiveEligible"] = emitter.reactiveEligible;
    json["preemptiveEligible"] = emitter.preemptiveEligible;
    json["consentRequired"] = emitter.consentRequired;
    json["operatorManaged"] = emitter.operatorManaged;
    json["jam"] = emitter.jam;
    json["jamIneffective"] = emitter.jamIneffective;
    json["jamEffective"] = emitter.jamEffective;
    QJsonDocument doc(json);
    return doc.toJson(QJsonDocument::Compact).toStdString() + "\n";
}

/*!
    \fn std::string NetworkImplementation::serializeComplexBlob(const PE& pe, const Emitter& emitter, const std::map<std::string, double>& doubleMap)
    \brief Serializes a complex blob containing a PE, an Emitter, and a map of doubles to a JSON string.
    \param pe The PE object to include in the blob.
    \param emitter The Emitter object to include in the blob.
    \param doubleMap A map of string keys to double values to include in the blob.
    \return A JSON string representation of the complex blob.
*/
std::string NetworkImplementation::serializeComplexBlob(const PE& pe, const Emitter& emitter, const std::map<std::string, double>& doubleMap) {
    QJsonObject json;
    json["pe"] = QJsonObject{{"data", QString::fromStdString(serializePE(pe))}};
    json["emitter"] = QJsonObject{{"data", QString::fromStdString(serializeEmitter(emitter))}};
    QJsonObject mapJson;
    for (const auto& pair : doubleMap) {
        mapJson[QString::fromStdString(pair.first)] = pair.second;
    }
    json["doubleMap"] = mapJson;
    QJsonDocument doc(json);
    return doc.toJson(QJsonDocument::Compact).toStdString() + "\n";
}

/*!
    \fn PE NetworkImplementation::deserializePE(const std::string& data)
    \brief Deserializes a JSON string to a PE object.
    \param data The JSON string to deserialize.
    \return A PE object created from the JSON data.
*/
PE NetworkImplementation::deserializePE(const std::string& data) {
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(data).toUtf8());
    if (doc.isNull()) {
        throw std::runtime_error("Invalid JSON data for PE deserialization");
    }
    QJsonObject json = doc.object();

    QStringList requiredFields = {"id", "type", "lat", "lon", "altitude", "speed", "apd", "priority", "jam", "ghost"};
    // FIXME: Add data field validation

    PE pe(
        json["id"].toString(),
        json["type"].toString(),
        json["lat"].toDouble(),
        json["lon"].toDouble(),
        json["altitude"].toDouble(),
        json["speed"].toDouble(),
        json["apd"].toString(),
        json["priority"].toString(),
        json["jam"].toBool(),
        json["ghost"].toBool()
    );
    pe.heading = json["heading"].toDouble();
    pe.category = static_cast<PE::PECategory>(json["category"].toInt());
    pe.state = json["state"].toString();
    return pe;
}

/*!
    \fn std::vector<PE> NetworkImplementation::deserializePEs(const std::string& data)
    \brief Deserializes a JSON string to a vector of PE objects.
    \param data The JSON string to deserialize.
    \return A vector of PE objects created from the JSON data.
*/
std::vector<PE> NetworkImplementation::deserializePEs(const std::string& data) {
    std::vector<PE> pes;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(data).toUtf8());
    if (doc.isNull()) {
        throw std::runtime_error("Invalid JSON data for PE deserialization");
    }
    QJsonObject json = doc.object();

    QStringList requiredFields = {"id", "type", "lat", "lon", "altitude", "speed", "apd", "priority", "jam", "ghost"};
    // FIXME: Add data field validation

    PE pe(
        json["id"].toString(),
        json["type"].toString(),
        json["lat"].toDouble(),
        json["lon"].toDouble(),
        json["altitude"].toDouble(),
        json["speed"].toDouble(),
        json["apd"].toString(),
        json["priority"].toString(),
        json["jam"].toBool(),
        json["ghost"].toBool()
    );
    pe.heading = json["heading"].toDouble();
    pe.category = static_cast<PE::PECategory>(json["category"].toInt());
    pe.state = json["state"].toString();
    pes.push_back(pe);

    return pes;
}

/*!
    \fn std::vector<Emitter> NetworkImplementation::deserializeEmitters(const std::string& data)
    \brief Deserializes a JSON string to a vector of Emitter objects.
    \param data The JSON string to deserialize.
    \return A vector of Emitter objects created from the JSON data.
*/
std::vector<Emitter> NetworkImplementation::deserializeEmitters(const std::string& data) {
    std::vector<Emitter> emitters;
    std::istringstream iss(data);
    std::string line;
    while (std::getline(iss, line)) {
        QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(line).toUtf8());
        if (doc.isNull()) continue;
        QJsonObject json = doc.object();
        Emitter emitter(
            json["id"].toString(),
            json["type"].toString(),
            json["category"].toString(),
            json["lat"].toDouble(),
            json["lon"].toDouble(),
            json["freqMin"].toDouble(),
            json["freqMax"].toDouble(),
            json["active"].toBool(),
            json["eaPriority"].toString(),
            json["esPriority"].toString(),
            json["jamResponsible"].toBool(),
            json["reactiveEligible"].toBool(),
            json["preemptiveEligible"].toBool(),
            json["consentRequired"].toBool(),
            json["jam"].toBool()
        );
        emitter.altitude = json["altitude"].toDouble();
        emitter.heading = json["heading"].toDouble();
        emitter.speed = json["speed"].toDouble();
        emitter.operatorManaged = json["operatorManaged"].toBool();
        emitter.jamIneffective = json["jamIneffective"].toInt();
        emitter.jamEffective = json["jamEffective"].toInt();
        emitters.push_back(emitter);
    }
    return emitters;
}

/*!
    \fn std::tuple<PE, Emitter, std::map<std::string, double>> NetworkImplementation::deserializeComplexBlob(const std::string& data)
    \brief Deserializes a JSON string to a complex blob containing a PE, an Emitter, and a map of doubles.
    \param data The JSON string to deserialize.
    \return A tuple containing a PE object, an Emitter object, and a map of string keys to double values.
*/
std::tuple<PE, Emitter, std::map<std::string, double>> NetworkImplementation::deserializeComplexBlob(const std::string& data) {
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(data).toUtf8());
    QJsonObject json = doc.object();

    // Deserialize PE
    std::string peData = json["pe"].toObject()["data"].toString().toStdString();
    std::vector<PE> pes = deserializePEs(peData);
    if (pes.empty()) {
        throw std::runtime_error("Deserialization of 'pe' resulted in an empty object");
    }
    PE pe = pes[0];

    // Deserialize Emitter
    std::string emitterData = json["emitter"].toObject()["data"].toString().toStdString();
    std::vector<Emitter> emitters = deserializeEmitters(emitterData);
    if (emitters.empty()) {
        throw std::runtime_error("Deserialization of 'emitter' resulted in an empty object");
    }
    Emitter emitter = emitters[0];

    // Deserialize doubleMap
    std::map<std::string, double> doubleMap;
    QJsonObject mapJson = json["doubleMap"].toObject();
    for (auto it = mapJson.begin(); it != mapJson.end(); ++it) {
        doubleMap[it.key().toStdString()] = it.value().toDouble();
    }

    return std::make_tuple(pe, emitter, doubleMap);
}

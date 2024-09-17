#include "AbstractNetworkInterface.h"
#include "emitter.h"
#include "pe.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <iostream>
#include <sstream>
#include <stdexcept>

NetworkImplementation::NetworkImplementation()
    : socket(std::make_unique<boost::asio::ip::tcp::socket>(io_context)) {}

void NetworkImplementation::initialise(const std::string& address, unsigned short port) {
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(address), port);
    socket->connect(endpoint);
}

boost::asio::ip::tcp::socket* NetworkImplementation::getSocket() {
    return socket.get();
}

void NetworkImplementation::validateAndPrintDataBufferSize(std::string dataBuff, std::string funcName) {
    dataBuff.data() ? std::cout << funcName << " - Received intact data buffer of length: "
                                << dataBuff.length() << std::endl
                    : std::cout << "Received invalid data buffer of length: "
                                << dataBuff.length() << std::endl;
}

bool NetworkImplementation::sendBlob(const std::string& blobString) {
    boost::asio::write(*socket, boost::asio::buffer(blobString + "\n"));
    return true;
}

bool NetworkImplementation::sendPE(const PE& pe) {
    std::string data = serializePE(pe);
    boost::asio::write(*socket, boost::asio::buffer(data));
    return true;
}

bool NetworkImplementation::sendEmitter(const Emitter& emitter) {
    std::string data = serializeEmitter(emitter);
    boost::asio::write(*socket, boost::asio::buffer(data));
    return true;
}

bool NetworkImplementation::sendComplexBlob(const PE& pe, const Emitter& emitter, const std::map<std::string, double>& doubleMap) {
    std::string data = serializeComplexBlob(pe, emitter, doubleMap);
    std::cout << "SENDING COMPLEX BLOB:\n" << data << std::endl;
    boost::asio::write(*socket, boost::asio::buffer(data));
    return true;
}

std::vector<PE> NetworkImplementation::receivePEs() {
    boost::asio::streambuf buf;
    boost::asio::read_until(*socket, buf, '\n');
    std::string data{boost::asio::buffers_begin(buf.data()),
                     boost::asio::buffers_end(buf.data())};
    validateAndPrintDataBufferSize(data, "receivePEs");

    return deserializePEs(data);
}

std::vector<Emitter> NetworkImplementation::receiveEmitters() {
    boost::asio::streambuf buf;
    boost::asio::read_until(*socket, buf, '\n');
    std::string data{boost::asio::buffers_begin(buf.data()),
                     boost::asio::buffers_end(buf.data())};
    validateAndPrintDataBufferSize(data, "receiveEmitters");

    return deserializeEmitters(data);
}

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

std::tuple<PE, Emitter, std::map<std::string, double>> NetworkImplementation::receiveComplexBlob() {
    boost::asio::streambuf buf;
    boost::asio::read_until(*socket, buf, '\n');
    std::string data{boost::asio::buffers_begin(buf.data()),
                     boost::asio::buffers_end(buf.data())};
    validateAndPrintDataBufferSize(data, "receiveComplexBlob");
    return deserializeComplexBlob(data);
}

void NetworkImplementation::close() {
    socket->close();
}

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

// Note: Could update this to intake a vector of pes and emitters and serialize each into the blob before sending.
std::string NetworkImplementation::serializeComplexBlob(const PE& pe, const Emitter& emitter, const std::map<std::string, double>& doubleMap) {
    QJsonObject json;
    json["pe"] = QJsonDocument::fromJson(serializePE(pe).c_str()).object();
    json["emitter"] = QJsonDocument::fromJson(serializeEmitter(emitter).c_str()).object();

    QJsonObject mapJson;
    for (const auto& pair : doubleMap) {
        mapJson[QString::fromStdString(pair.first)] = pair.second;
    }
    json["doubleMap"] = mapJson;

    QJsonDocument doc(json);
    return doc.toJson(QJsonDocument::Compact).toStdString() + "\n";
}

std::vector<PE> NetworkImplementation::deserializePEs(const std::string& data) {
    std::vector<PE> pes;
    std::istringstream iss(data);
    std::string line;
    while (std::getline(iss, line)) {
        QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(line).toUtf8());
        if (doc.isNull()) continue;
        QJsonObject json = doc.object();
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
    }
    return pes;
}

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

// FIXME: Likely causing segfault on test run - deserialize pe not working when given as complex blob
std::tuple<PE, Emitter, std::map<std::string, double>> NetworkImplementation::deserializeComplexBlob(const std::string& data) {
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(data).toUtf8());
    QJsonObject json = doc.object();

    std::cout << data << std::endl;

    PE pe = deserializePEs(json["pe"].toObject().toVariantMap()["data"].toString().toStdString())[0];
    if (pe.id.isNull()) {
        throw std::runtime_error("Deserialization of 'pe' resulted in an empty object");
    }

    Emitter emitter = deserializeEmitters(json["emitter"].toObject().toVariantMap()["data"].toString().toStdString())[0];
    if (emitter.id.isNull()) {
        throw std::runtime_error("Deserialization of 'emitter' resulted in an empty object");
    }

    std::map<std::string, double> doubleMap;
    QJsonObject mapJson = json["doubleMap"].toObject();
    for (auto it = mapJson.begin(); it != mapJson.end(); ++it) {
        doubleMap[it.key().toStdString()] = it.value().toDouble();
    }

    return std::make_tuple(pe, emitter, doubleMap);
}

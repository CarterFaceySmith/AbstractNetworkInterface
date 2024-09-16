#include "AbstractNetworkInterface.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <sstream>

NetworkImplementation::NetworkImplementation()
    : socket(std::make_unique<boost::asio::ip::tcp::socket>(io_context)) {}

void NetworkImplementation::initialise(const std::string& address, unsigned short port) {
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(address), port);
    socket->connect(endpoint);
}

bool NetworkImplementation::sendPE(const PE& pe) {
    std::string data = serializePE(pe);
    boost::asio::write(*socket, boost::asio::buffer(data));
    return true;
}

std::vector<PE> NetworkImplementation::receivePEs() {
    boost::asio::streambuf buf;
    boost::asio::read_until(*socket, buf, '\n');
    std::string data{boost::asio::buffers_begin(buf.data()),
                     boost::asio::buffers_end(buf.data())};
    return deserializePEs(data);
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

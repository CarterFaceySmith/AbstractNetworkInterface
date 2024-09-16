#include "AbstractNetworkInterface.h"
#include <sstream>

NetworkImplementation::NetworkImplementation()
    : socket(std::make_unique<boost::asio::ip::tcp::socket>(io_context)) {}

void NetworkImplementation::initialise(const std::string& address, unsigned short port) {
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(address), port);
    socket->connect(endpoint);
}

bool NetworkImplementation::sendAirEntity(const AirEntity& entity) {
    std::string data = serializeAirEntity(entity);
    boost::asio::write(*socket, boost::asio::buffer(data));
    return true;
}

bool NetworkImplementation::sendDoubleMap(const std::map<double, double>& data) {
    std::string serialized = serializeDoubleMap(data);
    boost::asio::write(*socket, boost::asio::buffer(serialized));
    return true;
}

std::vector<AirEntity> NetworkImplementation::receiveAirEntities() {
    boost::asio::streambuf buf;
    boost::asio::read_until(*socket, buf, '\n');
    std::string data{boost::asio::buffers_begin(buf.data()), 
                     boost::asio::buffers_end(buf.data())};
    return deserializeAirEntities(data);
}

std::map<double, double> NetworkImplementation::receiveDoubleMap() {
    boost::asio::streambuf buf;
    boost::asio::read_until(*socket, buf, '\n');
    std::string data{boost::asio::buffers_begin(buf.data()), 
                     boost::asio::buffers_end(buf.data())};
    return deserializeDoubleMap(data);
}

void NetworkImplementation::close() {
    socket->close();
}

std::string NetworkImplementation::serializeAirEntity(const AirEntity& entity) {
    return std::to_string(entity.id) + "," +
           std::to_string(entity.x) + "," +
           std::to_string(entity.y) + "," +
           std::to_string(entity.z) + "," +
           std::to_string(entity.speed) + "," +
           entity.type + "\n";
}

std::string NetworkImplementation::serializeDoubleMap(const std::map<double, double>& data) {
    std::string result;
    for (const auto& pair : data) {
        result += std::to_string(pair.first) + ":" + std::to_string(pair.second) + ";";
    }
    result += "\n";
    return result;
}

std::vector<AirEntity> NetworkImplementation::deserializeAirEntities(const std::string& data) {
    std::vector<AirEntity> entities;
    std::istringstream iss(data);
    std::string line;
    while (std::getline(iss, line)) {
        std::istringstream lineStream(line);
        std::string field;
        AirEntity entity;
        
        std::getline(lineStream, field, ',');
        entity.id = std::stoi(field);
        
        std::getline(lineStream, field, ',');
        entity.x = std::stod(field);
        
        std::getline(lineStream, field, ',');
        entity.y = std::stod(field);
        
        std::getline(lineStream, field, ',');
        entity.z = std::stod(field);
        
        std::getline(lineStream, field, ',');
        entity.speed = std::stod(field);
        
        std::getline(lineStream, entity.type);
        
        entities.push_back(entity);
    }
    return entities;
}

std::map<double, double> NetworkImplementation::deserializeDoubleMap(const std::string& data) {
    std::map<double, double> result;
    std::istringstream iss(data);
    std::string pair;
    while (std::getline(iss, pair, ';')) {
        size_t colon = pair.find(':');
        if (colon != std::string::npos) {
            double key = std::stod(pair.substr(0, colon));
            double value = std::stod(pair.substr(colon + 1));
            result[key] = value;
        }
    }
    return result;
}

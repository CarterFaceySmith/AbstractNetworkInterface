#pragma once

#include <string>
#include <map>
#include <vector>
#include <boost/asio.hpp>
#include <memory>
#include "emitter.h"
#include "pe.h"

// Structure to represent an air entity in the game
struct AirEntity {
    int id;
    double x, y, z;  // 3D coordinates
    double speed;
    std::string type;  // e.g., "aircraft", "missile", "drone"
};

class AbstractNetworkInterface {
public:
    virtual ~AbstractNetworkInterface() = default;

    // Initialize the network connection
    virtual void initialise(const std::string& address, unsigned short port) = 0;

    // Send air entity data
    virtual bool sendAirEntity(const AirEntity& entity) = 0;

    // Send a map of paired doubles
    virtual bool sendDoubleMap(const std::map<double, double>& data) = 0;

    // Receive air entity data
    virtual std::vector<AirEntity> receiveAirEntities() = 0;

    // Receive a map of paired doubles
    virtual std::map<double, double> receiveDoubleMap() = 0;

    // Close the connection
    virtual void close() = 0;
};

class NetworkImplementation : public AbstractNetworkInterface {
public:
    NetworkImplementation();
    void initialise(const std::string& address, unsigned short port) override;
    bool sendAirEntity(const AirEntity& entity) override;
    bool sendDoubleMap(const std::map<double, double>& data) override;
    std::vector<AirEntity> receiveAirEntities() override;
    std::map<double, double> receiveDoubleMap() override;
    void close() override;

private:
    boost::asio::io_context io_context;
    std::unique_ptr<boost::asio::ip::tcp::socket> socket;

    std::string serializeAirEntity(const AirEntity& entity);
    std::string serializeDoubleMap(const std::map<double, double>& data);
    std::vector<AirEntity> deserializeAirEntities(const std::string& data);
    std::map<double, double> deserializeDoubleMap(const std::string& data);
};

#pragma once

#include <vector>
#include <boost/asio.hpp>
#include <memory>
#include "pe.h"

#ifndef ABSTRACTNETWORKINTERFACE_H
#define ABSTRACTNETWORKINTERFACE_H

class AbstractNetworkInterface {
public:
    virtual ~AbstractNetworkInterface() = default;

    // Initialize the network connection
    virtual void initialise(const std::string& address, unsigned short port) = 0;

    // Send air entity data
    virtual bool sendPE(const PE& pe) = 0;

    // Receive air entity data
    virtual std::vector<PE> receivePEs() = 0;

    // Close the connection
    virtual void close() = 0;
};

class NetworkImplementation : public AbstractNetworkInterface {
public:
    NetworkImplementation();
    ~NetworkImplementation() override = default;
    boost::asio::ip::tcp::socket* getSocket();

    void initialise(const std::string& address, unsigned short port) override;
    bool sendPE(const PE& pe) override;
    std::vector<PE> receivePEs() override;
    void close() override;

private:
    std::string serializePE(const PE& pe);
    std::vector<PE> deserializePEs(const std::string& data);

    boost::asio::io_context io_context;
    std::unique_ptr<boost::asio::ip::tcp::socket> socket;
};

#endif // ABSTRACTNETWORKINTERFACE_h

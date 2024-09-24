#pragma once

#include <vector>
#include <boost/asio.hpp>
#include <memory>
#include <map>
#include <tuple>
#include "pe.h"
#include "emitter.h"

#ifndef ABSTRACTNETWORKINTERFACE_H
#define ABSTRACTNETWORKINTERFACE_H

class AbstractNetworkInterface {
public:
    virtual ~AbstractNetworkInterface() = default;
    // Initialize the network connection
    virtual void initialise(const std::string& address, unsigned short port) = 0;
    // Send air entity data
    virtual bool sendPE(const PE& pe) = 0;
    // Send emitter data
    virtual bool sendEmitter(const Emitter& emitter) = 0;
    // Send entire generic data blob
    virtual bool sendBlob(const std::string& blobString) = 0;
    // Send complex blob (PE, Emitter, and map of doubles)
    virtual bool sendComplexBlob(const PE& pe, const Emitter& emitter, const std::map<std::string, double>& doubleMap) = 0;
    // Send single update setting for a PE
    virtual bool sendPESetting(const std::string& setting, const std::string& id, int updateVal) = 0;
    // Send single update setting for an Emitter
    virtual bool sendEmitterSetting(const std::string& setting, const std::string& id, int updateVal) = 0;
    // Receives and constructs a tuple of a given emitter or pe setting
    virtual std::tuple<std::string, std::string, std::string, int> receiveSetting() = 0;
    // Receive air entity data
    virtual PE receivePE() = 0;
    // Receive emitter data
    virtual Emitter receiveEmitter() = 0;
    // Receive entire data blob
    virtual std::vector<std::string> receiveBlob() = 0;
    // Receive complex blob (PE, Emitter, and map of doubles)
    virtual std::tuple<PE, Emitter, std::map<std::string, double>> receiveComplexBlob() = 0;
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
    bool sendEmitter(const Emitter& emitter) override;
    bool sendBlob(const std::string& blobString) override;
    bool sendComplexBlob(const PE& pe, const Emitter& emitter, const std::map<std::string, double>& doubleMap) override;
    bool sendPESetting(const std::string& setting, const std::string& id, int updateVal) override;
    std::tuple<std::string, std::string, std::string, int> receiveSetting() override;
    bool sendEmitterSetting(const std::string& setting, const std::string& id, int updateVal) override;
    PE receivePE() override;
    Emitter receiveEmitter() override;
    std::vector<std::string> receiveBlob() override;
    std::tuple<PE, Emitter, std::map<std::string, double>> receiveComplexBlob() override;
    void validateAndPrintDataBufferSize(std::string dataBuff, std::string funcName);
    void close() override;

private:
    std::string serializePE(const PE& pe);
    std::string serializeEmitter(const Emitter& emitter);
    std::string serializeComplexBlob(const PE& pe, const Emitter& emitter, const std::map<std::string, double>& doubleMap);
    PE deserializePE(const std::string& data);
    Emitter deserializeEmitter(const std::string& data);
    std::tuple<PE, Emitter, std::map<std::string, double>> deserializeComplexBlob(const std::string& data);
    boost::asio::io_context io_context;
    std::unique_ptr<boost::asio::ip::tcp::socket> socket;
    bool validatePE(const PE& pe);
    bool validateEmitter(const Emitter& emitter);
    void logError(const std::string& message);
};

#endif // ABSTRACTNETWORKINTERFACE_H

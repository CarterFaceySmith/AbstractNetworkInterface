#include <gtest/gtest.h>
#include "AbstractNetworkInterface.h"
#include <thread>
#include <chrono>
#include <map>
#include <iostream>

class NetworkImplementationTest : public ::testing::Test {
protected:
    std::unique_ptr<NetworkImplementation> server;
    std::unique_ptr<NetworkImplementation> client;

    void SetUp() override {
        std::cout << "Setting up test..." << std::endl;
        server = std::make_unique<NetworkImplementation>();
        client = std::make_unique<NetworkImplementation>();

        // Start server in a separate thread
        std::thread serverThread([this]() {
            try {
                boost::asio::io_context io_context;
                boost::asio::ip::tcp::acceptor acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 3525));
                acceptor.accept(*(static_cast<boost::asio::ip::tcp::socket*>(server->getSocket())));
                std::cout << "Server accepted connection" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Server thread exception: " << e.what() << std::endl;
            }
        });

        // Give the server a moment to start
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        try {
            // Connect client
            client->initialise("127.0.0.1", 3525);
            std::cout << "Client connected" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Client connection exception: " << e.what() << std::endl;
        }

        serverThread.join();
        std::cout << "Test setup completed" << std::endl;
    }

    void TearDown() override {
        std::cout << "Tearing down test..." << std::endl;
        server->close();
        client->close();
        std::cout << "Test tear down completed" << std::endl;
    }
};

TEST_F(NetworkImplementationTest, SendReceivePE) {
    std::cout << "Starting SendReceivePE test" << std::endl;
    PE sentPE("TestID", "F18", 10.0, 20.0, 30000.0, 500.0, "MED", "HIGH", false, false);
    ASSERT_TRUE(client->sendPE(sentPE));
    std::cout << "PE sent" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Allow time for message to be sent
    auto receivedPEs = server->receivePEs();
    std::cout << "PE received" << std::endl;
    ASSERT_EQ(receivedPEs.size(), 1);
    PE receivedPE = receivedPEs[0];
    EXPECT_EQ(receivedPE.id, sentPE.id);
    EXPECT_EQ(receivedPE.type, sentPE.type);
    EXPECT_DOUBLE_EQ(receivedPE.lat, sentPE.lat);
    EXPECT_DOUBLE_EQ(receivedPE.lon, sentPE.lon);
    EXPECT_DOUBLE_EQ(receivedPE.altitude, sentPE.altitude);
    EXPECT_DOUBLE_EQ(receivedPE.speed, sentPE.speed);
    EXPECT_EQ(receivedPE.apd, sentPE.apd);
    EXPECT_EQ(receivedPE.priority, sentPE.priority);
    EXPECT_EQ(receivedPE.jam, sentPE.jam);
    EXPECT_EQ(receivedPE.ghost, sentPE.ghost);
    std::cout << "SendReceivePE test completed" << std::endl;
}

TEST_F(NetworkImplementationTest, SendReceiveEmitter) {
    std::cout << "Starting SendReceiveEmitter test" << std::endl;
    Emitter sentEmitter("EmitterID", "RadarType", "Category", 15.0, 25.0, 8.0, 12.0);
    ASSERT_TRUE(client->sendEmitter(sentEmitter));
    std::cout << "Emitter sent" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Allow time for message to be sent
    auto receivedEmitters = server->receiveEmitters();
    std::cout << "Emitter received" << std::endl;
    ASSERT_EQ(receivedEmitters.size(), 1);
    Emitter receivedEmitter = receivedEmitters[0];
    EXPECT_EQ(receivedEmitter.id, sentEmitter.id);
    EXPECT_EQ(receivedEmitter.type, sentEmitter.type);
    EXPECT_EQ(receivedEmitter.category, sentEmitter.category);
    EXPECT_DOUBLE_EQ(receivedEmitter.lat, sentEmitter.lat);
    EXPECT_DOUBLE_EQ(receivedEmitter.lon, sentEmitter.lon);
    EXPECT_DOUBLE_EQ(receivedEmitter.freqMin, sentEmitter.freqMin);
    EXPECT_DOUBLE_EQ(receivedEmitter.freqMax, sentEmitter.freqMax);
    std::cout << "SendReceiveEmitter test completed" << std::endl;
}

TEST_F(NetworkImplementationTest, SendReceiveComplexBlob) {
    std::cout << "Starting SendReceiveComplexBlob test" << std::endl;
    PE sentPE("TestID", "F18", 10.0, 20.0, 30000.0, 500.0, "MED", "HIGH", false, false);
    Emitter sentEmitter("EmitterID", "RadarType", "Category", 15.0, 25.0, 8.0, 12.0);
    std::map<std::string, double> sentDoubleMap = {{"key1", 1.0}, {"key2", 2.0}};

    std::cout << "Sending complex blob" << std::endl;
    ASSERT_TRUE(client->sendComplexBlob(sentPE, sentEmitter, sentDoubleMap));
    std::cout << "Complex blob sent" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Allow time for message to be sent

    // FIXME: Causes segmentation fault
    std::cout << "Receiving complex blob" << std::endl;

    try {
        auto [receivedPE, receivedEmitter, receivedDoubleMap] = server->receiveComplexBlob();
        std::cout << "Complex blob received successfully" << std::endl;

        // Check PE
        EXPECT_EQ(receivedPE.id, sentPE.id);
        EXPECT_EQ(receivedPE.type, sentPE.type);
        EXPECT_DOUBLE_EQ(receivedPE.lat, sentPE.lat);
        EXPECT_DOUBLE_EQ(receivedPE.lon, sentPE.lon);

        // Check Emitter
        EXPECT_EQ(receivedEmitter.id, sentEmitter.id);
        EXPECT_EQ(receivedEmitter.type, sentEmitter.type);
        EXPECT_DOUBLE_EQ(receivedEmitter.lat, sentEmitter.lat);
        EXPECT_DOUBLE_EQ(receivedEmitter.lon, sentEmitter.lon);

        // Check Double Map
        EXPECT_EQ(receivedDoubleMap.size(), sentDoubleMap.size());
        for (const auto& [key, value] : sentDoubleMap) {
            EXPECT_DOUBLE_EQ(receivedDoubleMap[key], value);
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        FAIL() << "Exception thrown during complex blob reception";
    }

    std::cout << "SendReceiveComplexBlob test completed" << std::endl;
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

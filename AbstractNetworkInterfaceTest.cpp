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
    PE receivedPE = server->receivePE();
    std::cout << "PE received" << std::endl;
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
    Emitter sentEmitter("EmitterID", "RadarType", "Category", 15.0, 25.0, 8000.0, 12000.0, false);
    ASSERT_TRUE(client->sendEmitter(sentEmitter));
    std::cout << "Emitter sent" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Allow time for message to be sent
    Emitter receivedEmitter = server->receiveEmitter();
    std::cout << "Emitter received" << std::endl;
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

TEST_F(NetworkImplementationTest, SendInvalidPE) {
    PE invalidPE("", "", -1.0, -1.0, -1.0, -1.0, "", "", false, false);
    EXPECT_FALSE(client->sendPE(invalidPE));
}

TEST_F(NetworkImplementationTest, SendInvalidEmitter) {
    Emitter invalidEmitter("", "", "", -1.0, -1.0, -1.0, -1.0);
    EXPECT_FALSE(client->sendEmitter(invalidEmitter));
}

TEST_F(NetworkImplementationTest, PerformanceTestPEs) {
    std::cout << "Starting PerformanceTestPEs test" << std::endl;
    const int numMessages = 20;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numMessages; ++i) {
        std::string id = "TestID" + std::to_string(i);
        PE sentPE(id.c_str(), "F18", 10.0, 20.0, 30000.0, 500.0, "MED", "HIGH", false, false);
        ASSERT_TRUE(client->sendPE(sentPE));
        std::cout << "Sent PE " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Short delay between sends
        PE receivedPE = server->receivePE();
        std::cout << "Received PE " << receivedPE.id.toStdString() << std::endl;
        std::string idToCheck = "TestID" + std::to_string(i);
        EXPECT_EQ(receivedPE.id, idToCheck.c_str());
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));  // Allow time for all messages to be received

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Time taken to send and receive " << numMessages << " PEs: " << duration.count() << "ms" << std::endl;

        std::cout << "PerformanceTestPEs test completed" << std::endl;

}

TEST_F(NetworkImplementationTest, PerformanceTestEmitters) {
    std::cout << "Starting PerformanceTestEmitters test" << std::endl;
    const int numMessages = 20;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numMessages; ++i) {
        std::string id = "TestID" + std::to_string(i);
        Emitter sentEmitter(id.c_str(), "RadarType", "Category", 15.0, 25.0, 8.0, 12.0);
        ASSERT_TRUE(client->sendEmitter(sentEmitter));
        std::cout << "Sent Emitter " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Short delay between sends
        Emitter receivedEmitter = server->receiveEmitter();
        std::cout << "Received Emitter " << receivedEmitter.id.toStdString() << std::endl;
        std::string idToCheck = "TestID" + std::to_string(i);
        EXPECT_EQ(receivedEmitter.id, idToCheck.c_str());
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));  // Allow time for all messages to be received

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Time taken to send and receive " << numMessages << " Emitters: " << duration.count() << "ms" << std::endl;

    std::cout << "PerformanceTestEmitters test completed" << std::endl;

}

TEST_F(NetworkImplementationTest, SendReceivePESetting) {
    std::cout << "Starting SendReceivePESetting test" << std::endl;
    std::string id = "PE001";
    std::string setting = "APD";
    int value = 5;

    ASSERT_TRUE(client->sendPESetting(setting, id, value));
    std::cout << "PE setting sent" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Allow time for message to be sent

    auto [type, receivedId, receivedSetting, receivedValue] = server->receiveSetting();
    std::cout << "Setting received" << std::endl;

    EXPECT_EQ(type, "PE_SETTING");
    EXPECT_EQ(receivedId, id);
    EXPECT_EQ(receivedSetting, setting);
    EXPECT_EQ(receivedValue, value);

    std::cout << "SendReceivePESetting test completed" << std::endl;
}

TEST_F(NetworkImplementationTest, SendReceiveEmitterSetting) {
    std::cout << "Starting SendReceiveEmitterSetting test" << std::endl;
    std::string id = "EM001";
    std::string setting = "PRIO";
    int value = 2;

    ASSERT_TRUE(client->sendEmitterSetting(setting, id, value));
    std::cout << "Emitter setting sent" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Allow time for message to be sent

    auto [type, receivedId, receivedSetting, receivedValue] = server->receiveSetting();
    std::cout << "Setting received" << std::endl;

    EXPECT_EQ(type, "EMITTER_SETTING");
    EXPECT_EQ(receivedId, id);
    EXPECT_EQ(receivedSetting, setting);
    EXPECT_EQ(receivedValue, value);

    std::cout << "SendReceiveEmitterSetting test completed" << std::endl;
}

TEST_F(NetworkImplementationTest, PerformanceTestSettings) {
    std::cout << "Starting PerformanceTestSettings test" << std::endl;
    const int numMessagesPerEntity = 20;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numMessagesPerEntity; ++i) {
        // Send PE setting
        ASSERT_TRUE(client->sendPESetting("APD", "PE00" + std::to_string(i), 10));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        auto [type, id, setting, value] = server->receiveSetting();
        if (type == "PE_SETTING") {
            EXPECT_EQ(id, "PE00" + std::to_string(i));
            EXPECT_EQ(setting, "APD");
            EXPECT_EQ(value, 10);
        } else if (type == "EMITTER_SETTING") {
            EXPECT_EQ(id, "EM00" + std::to_string(i));
            EXPECT_EQ(setting, "PRIO");
            EXPECT_EQ(value, 10);
        } else {
            FAIL() << "Unexpected setting type received";
        }

        // Send Emitter setting
        ASSERT_TRUE(client->sendEmitterSetting("PRIO", "EM00" + std::to_string(i), 10));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        auto [type2, id2, setting2, value2] = server->receiveSetting();
        if (type2 == "PE_SETTING") {
            EXPECT_EQ(id2, "PE00" + std::to_string(i));
            EXPECT_EQ(setting2, "APD");
            EXPECT_EQ(value2, 10);
        } else if (type2 == "EMITTER_SETTING") {
            EXPECT_EQ(id2, "EM00" + std::to_string(i));
            EXPECT_EQ(setting2, "PRIO");
            EXPECT_EQ(value2, 10);
        } else {
            FAIL() << "Unexpected setting type received";
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));  // Allow time for all messages to be received

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Time taken to send and receive " << numMessagesPerEntity << " settings per entity: " << duration.count() << "ms" << std::endl;

    std::cout << "PerformanceTestSettings test completed" << std::endl;
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

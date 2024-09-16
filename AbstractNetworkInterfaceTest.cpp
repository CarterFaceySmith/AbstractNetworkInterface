#include <gtest/gtest.h>
#include "AbstractNetworkInterface.h"
#include <thread>
#include <chrono>

class AbstractNetworkInterfaceTest : public ::testing::Test {
protected:
    std::unique_ptr<TCPInterface> server;
    std::unique_ptr<TCPInterface> client;

    void SetUp() override {
        server = std::make_unique<TCPImplementation>();
        client = std::make_unique<TCPImplementation>();

        // Start server in a separate thread
        std::thread serverThread([this]() {
            boost::asio::io_context io_context;
            boost::asio::ip::tcp::acceptor acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 12345));
            acceptor.accept(*server->socket);
        });

        // Give the server a moment to start
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Connect client
        client->initialize("127.0.0.1", 12345);

        serverThread.join();
    }

    void TearDown() override {
        server->close();
        client->close();
    }
};

TEST_F(AbstractNetworkInterfaceTest, SendReceiveAirEntity) {
    AirEntity sentEntity{1, 10.0, 20.0, 30.0, 500.0, "fighter"};
    
    ASSERT_TRUE(client->sendAirEntity(sentEntity));

    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Allow time for message to be sent

    auto receivedEntities = server->receiveAirEntities();
    ASSERT_EQ(receivedEntities.size(), 1);

    AirEntity receivedEntity = receivedEntities[0];
    EXPECT_EQ(receivedEntity.id, sentEntity.id);
    EXPECT_DOUBLE_EQ(receivedEntity.x, sentEntity.x);
    EXPECT_DOUBLE_EQ(receivedEntity.y, sentEntity.y);
    EXPECT_DOUBLE_EQ(receivedEntity.z, sentEntity.z);
    EXPECT_DOUBLE_EQ(receivedEntity.speed, sentEntity.speed);
    EXPECT_EQ(receivedEntity.type, sentEntity.type);
}

TEST_F(AbstractNetworkInterfaceTest, SendReceiveDoubleMap) {
    std::map<double, double> sentMap = {{1.0, 2.0}, {3.0, 4.0}, {5.0, 6.0}};
    
    ASSERT_TRUE(client->sendDoubleMap(sentMap));

    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Allow time for message to be sent

    auto receivedMap = server->receiveDoubleMap();
    EXPECT_EQ(receivedMap.size(), sentMap.size());

    for (const auto& pair : sentMap) {
        EXPECT_TRUE(receivedMap.find(pair.first) != receivedMap.end());
        EXPECT_DOUBLE_EQ(receivedMap[pair.first], pair.second);
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
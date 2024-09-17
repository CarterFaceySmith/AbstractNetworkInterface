#include <gtest/gtest.h>
#include "AbstractNetworkInterface.h"
#include <thread>
#include <chrono>

class NetworkImplementationTest : public ::testing::Test {
protected:
    std::unique_ptr<NetworkImplementation> server;
    std::unique_ptr<NetworkImplementation> client;

    void SetUp() override {
        server = std::make_unique<NetworkImplementation>();
        client = std::make_unique<NetworkImplementation>();

        // Start server in a separate thread
        std::thread serverThread([this]() {
            boost::asio::io_context io_context;
            boost::asio::ip::tcp::acceptor acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 3525));
            acceptor.accept(*(static_cast<boost::asio::ip::tcp::socket*>(server->getSocket())));
        });

        // Give the server a moment to start
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Connect client
        client->initialise("127.0.0.1", 3525);

        serverThread.join();
    }

    void TearDown() override {
        server->close();
        client->close();
    }
};

TEST_F(NetworkImplementationTest, SendReceivePE) {
    PE sentPE("TestID", "F18", 10.0, 20.0, 30000.0, 500.0, "MED", "HIGH", false, false);

    ASSERT_TRUE(client->sendPE(sentPE));
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Allow time for message to be sent

    auto receivedPEs = server->receivePEs();
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
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

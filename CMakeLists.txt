cmake_minimum_required(VERSION 3.14)
project(CarterMessage VERSION 1.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Quick Qml Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Quick Qml Gui)
find_package(Boost REQUIRED COMPONENTS system)

# Add the TCP interface library
add_library(AbstractNetworkInterface STATIC AbstractNetworkInterface.cpp)
target_link_libraries(AbstractNetworkInterface PUBLIC Boost::system)

# Main application
add_executable(CarterMessage
    main.cpp
)

target_link_libraries(CarterMessage
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Gui
    AbstractNetworkInterface
)

# # Google Test
# include(FetchContent)
# FetchContent_Declare(
#   googletest
#   URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
# )
# FetchContent_MakeAvailable(googletest)

# Add the test executable
add_executable(run_tests
    AbstractNetworkInterface.cpp
    AbstractNetworkInterface.h
    main.cpp
    pe.h
    emitter.h
)

target_link_libraries(run_tests
    PRIVATE
    AbstractNetworkInterface
    gtest
    gtest_main
    Boost::system
    Qt${QT_VERSION_MAJOR}::Core
)

include(GNUInstallDirs)
install(TARGETS CarterMessage
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Add the test
include(CTest)
add_test(NAME AbstractNetworkInterfaceTest COMMAND run_tests)

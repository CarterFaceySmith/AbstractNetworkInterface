cmake_minimum_required(VERSION 3.14)
project(CarterMessage VERSION 1.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add option to disable gtest
option(ENABLE_GTEST "Enable Google Test framework" ON)

# Find required packages
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Quick)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Quick)
find_package(Boost REQUIRED COMPONENTS system)

# Add the TCP interface library
add_library(AbstractNetworkInterface STATIC
    AbstractNetworkInterface.cpp
    AbstractNetworkInterface.h
)

target_link_libraries(AbstractNetworkInterface
    PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Boost::system
)

# Main application
add_executable(CarterMessage
    main.cpp
    pe.h
    emitter.h
)

# Include Google Test if enabled
if(ENABLE_GTEST)
    include(FetchContent)
    FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG release-1.11.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(AbstractNetworkInterfaceTest
        AbstractNetworkInterfaceTest.cpp
    )

    target_link_libraries(AbstractNetworkInterfaceTest
        PRIVATE
        AbstractNetworkInterface
        gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(AbstractNetworkInterfaceTest)
endif()

# Link Qt libraries and AbstractNetworkInterface
target_link_libraries(CarterMessage
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Quick
    AbstractNetworkInterface
)

# QML-specific configurations
set_target_properties(CarterMessage PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_import_qml_plugins(CarterMessage)
    qt_finalize_executable(CarterMessage)
endif()

include(GNUInstallDirs)
install(TARGETS CarterMessage
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

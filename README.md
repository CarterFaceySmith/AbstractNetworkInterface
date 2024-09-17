<h3 align="center">Carter's Abstract Network Interface</h3>
<br>
<p align="center"><i>An abstract networking interface class for use in QML applications, implementing aerospace entities</i></p>

## About The Project

This project provides an abstract networking interface class written in C++ designed for integration with QML applications. It aims to facilitate the development of networking components that model aerospace entities.

## Pre-requisites

Before you can build and test the project, ensure that you have the following dependencies installed:

- `boost`
- `gtest` (Google Test)

## Compiling and Running Tests

Follow these steps to compile the project and run tests:

1. **Optional: Clean Previous Builds**: 
    ```bash
    rm -rf build
    ```

2. **Optional: Create Build Directory**: 
    ```bash
    mkdir build
    ```

3. **Navigate to Build Directory**: 
    ```bash
    cd build
    ```

4. **Generate Build Files**: 
    ```bash
    cmake ..
    ```

5. **Compile the Project**: 
    ```bash
    make
    ```

6. **Run Tests**: 
    ```bash
    ctest
    ```

7. **Alternatively, Run the Test Executable Directly**: 
    ```bash
    ./AbstractNetworkInterfaceTest
    ```

### Notes

If you encounter any issues or have questions about the project, please feel free to [contact me](mailto:carterfs@proton.me).

## My-Private-SDK
A lightweight SDK designed for C++ developers working in **cross-compilation environments**.  
It provides utility wrappers around standard C++ libraries (e.g., `<chrono>`) to simplify usage and enhance productivity.

> This project is updated infrequently, but we aim to keep it maintained and relevant.

Contributions and suggestions are welcome.

---

## Test Environment & Requirements

- **Platform**: Designed for Linux systems (Debian-based), e.g. Ubuntu
- **IDE**: Visual Studio Code (v1.79.2)
- **CMake**: Version 3.0 or higher
- **C++ Standard**: C++17 or later
- **Compiler Requirements**:
    - Must support C++17 features such as `std::filesystem`, `std::is_same_v`
    - ⚠️ **Recommended version**: GCC 11.4+ or MSVC 14.29+

- **Tested On**: Ubuntu 22.04 LTS, JetPack 5.1.2 (L4T 18.04), Windows 11

    |          | Linux system (ARM 64)  | Windows system (AMD 64) |
    | :---:    | :---:                  | :---:                   |
    | System   | Jetpack 5.1.2          | Windows 11 Pro          |
    | Detail   | L4T (ubuntu 18.04 LTS) | 22H2. 22621.1848        |
    | Compiler | GCC 11.4.0             | MSVC 14.29.30133        |

## How to Build from Source

1. **Clone the repository**

    ```bash
    git clone https://github.com/lasiyan/Rowen-SDK.git
    cd Rowen-SDK
    ```

2. **Customize your build (optional)**

    - Specify a compiler:
      ```bash
      cmake -DCMAKE_CXX_COMPILER=g++-13 ..
      ```
    - Set a custom installation path:
      ```bash
      # default install path is "{REPOSITORY_PATH}/install"
      cmake -DCMAKE_INSTALL_PREFIX=/your/path ..
      ```

3. **Build and install**

    ```bash
    mkdir build
    cd build
    cmake ..   # Use additional options as needed
    make -j$(nproc)
    make install
    ```

4. **Installation output**

    The default install path will be `./install`. Copy the `rowen-sdk` folder into your target project.

5. **Project configuration example (CMake)**

    ```cmake
    target_include_directories(MYAPP PUBLIC rowen-sdk/include)
    target_link_directories(MYAPP PUBLIC rowen-sdk/lib)
    target_link_libraries(MYAPP PUBLIC rowen_core rowen_utils)
    ```

6. **Sample usage**

    ```cpp
    #include <rowen/core.hpp>

    int main() {
        auto str1 = rs::format("%s", "hello world!");
        logger.info("%s", str1.c_str());

        auto str2 = rs::Time::timeString();
        logger.info("Current Time: %s", str2.c_str());

        return 0;
    }
    ```

---

## 🧩 External Dependencies (Bundled)

This SDK internally wraps and simplifies several popular open-source C++ libraries:

- **jsoncpp**
- **rapidjson**
- **yaml-cpp**

The required source (`.cpp`) and header (`.h`) files for these libraries are **included directly in this repository**.  
You do **not** need to install them separately.

Additionally, the SDK provides **helper functions** that abstract and simplify common usage patterns of these libraries.

> ⚠️ The original open-source licenses for each library are preserved and respected in the repository.

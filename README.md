## My-Private-SDK
This is aimed at C++ users working in a cross-compilation environment.
It also wrap standard C++ libraries (like Chrono) to make them easier for users to use.

This will be updated infrequently, but we will keep it up to date.

---

## Test Environment
- **IDE** : Visual studio Code (1.79.2)
- **Target** : CXX Standard version 17
- **CMake** : Required version 3.0

|          | Linux system (ARM 64)  | Windows system (AMD 64) |
| :---:    | :---:                  | :---:                   |
| System   | Jetpack 5.1.2          | Windows 11 Pro          |
| Detail   | L4T (ubuntu 18.04 LTS) | 22H2. 22621.1848        |
| Compiler | GCC 13.1.0             | MSVC 14.29.30133        |

## How to use (from. Source)

1. Clone this repository to your desktop

    ```bash
    $ git clone https://github.com/lasiyan/Rowen-SDK.git
    $ cd Rowen-SDK
    ```

2. (Optional) Choose your own configure options

    - If you want to specify a compiler, use the `CMAKE_CXX_COMPILER` flag.
    - If you want to install to a custom location, use `CMAKE_INSTALL_PREFIX`.

3. Build and install the source

    ```bash
    mkdir build
    cd build
    cmake .. # See step 2
    make -j4
    make install
    ```

    If you want to use your own compiler, set it like below

    ```bash
    # Alternatively, "cmake .."
    cmake -DCMAKE_CXX_COMPILER=g++-9 ..
    ```

4. Installing with the default path will create an `install` folder. Copy this `rowen-sdk` folder to your project.

5. Then configure your project as follows

    ```cmake
    target_include_directories(MYAPP PUBLIC rowen-sdk/include)
    target_link_directories(MYAPP PUBLIC rowen-sdk/lib)
    target_link_libraries(MYAPP PUBLIC rowen_core rowen_utils)
    ```

6. Sample

    ```cpp
    #include <rowen/core.hpp>

    int main()
    {
      auto str1 = rs::format("%s", "hello world !");
      logger.info("%s", str1.c_str());

      auto str2 = rs::Time::timeString();
      logger.info("Current Time: %s", str2.c_str());

      return 0;
    }
    ```

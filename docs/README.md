# CPPCMake Project

## Project's Purpose
The CPPCMake project aims to provide a modern C++ build system using CMake, integrating tools for code formatting, linting, and automated testing. It leverages CircleCI for continuous integration and delivery, ensuring code quality and build reliability.

## Features
- **Modern C++ Standards**: Utilizes C++20 standards for development.
- **Automated Code Formatting and Linting**: Integrates Clang-Format and Clang-Tidy to ensure code quality.
- **Continuous Integration**: Configured with CircleCI to automate the build and test process.
- **Link-Time Optimization Support**: Checks for and enables LTO if supported by the compiler.
- **Conditional Compilation**: Supports conditional compilation based on the availability of `ppoll()` and `pselect()`.
- **Automated Testing**: Includes a test suite that can be run with CTest.
- **Cross-Platform Compatibility**: Ensures compatibility with various operating systems, including checks for specific features like `fork()` and `pipe()` for browse mode support.

## Requirements
Before installing the CPPCMake project, ensure you have the following prerequisites installed on your system:
- **Operating System**: Linux, macOS, or Windows.
- **CMake**: Version 3.15 or higher.
- **Compiler**: A C++ compiler supporting C++20 (GCC, Clang, MSVC).
- **Clang-Format and Clang-Tidy**: For code formatting and static analysis.
- **Google Test**: For unit testing (optional, as CMake can fetch it automatically).

**For more information on setup and configuration, refer to the `CMakeLists.txt` and `.circleci/config.yml` files.**
## Installation Instructions
Before installing the CPPCMake project, ensure you have the following prerequisites installed on your system:
1. **Prerequisites**:
    - CMake version 3.15 or higher.
    - A C++ compiler supporting C++20.
    - Clang-Format and Clang-Tidy for code formatting and linting.
    - GTest for running the test suite.

2. **Building the Project**:
- **Clone the repository**:
```bash 
git clone https://github.com/aniket-ray/cppcmake.git
cd cppcmake
```
- **Create a build directory**:
This directory is where CMake will generate build files.
```bash
mkdir build && cd build
```
- **Configure the project**:
Run CMake to configure the project. You can specify additional options to customize the build. For example, to force the use of pselect() over ppoll() (if both are available), add -DCPPCMAKE_FORCE_PSELECT=ON to the command.
```bash
cmake ..
```
- **Build the project**: Compile the project. You can specify the -j option to parallelize the build process and speed it up.
```bash
cmake --build . --config Release
```
- **Run tests (Optional)**: If you have Google Test installed or allowed CMake to fetch it, you can run the unit tests compiled as part of the project.
```bash
ctest
```

These instructions should help you get the CPPCMake project up and running on your system. Adjustments may be necessary based on your specific environment or if you encounter any issues related to dependencies not covered here.


## Usage Instructions
1. **Running the Application**:
    - After building the project, you can run the generated binary to see it in action
    ```bash
    ./cppcmake
   ```
2. **Running the Application**:
    - To run the test suite, use the following command from the build directory
    ```bash
    ctest
    ```

## Contribution Guidelines
1. **Fork the repository**: Click the "Fork" button on the GitHub repository page.
2. **Create a new branch for your feature**: `git checkout -b new-feature`
3. **Commit your changes with clear, descriptive commit messages**: `git commit -am 'Add some feature`
4. **Push to the Branch**: `git push origin new-feature`
5. **Submit a Pull Request**: Open a new pull request from your feature branch to the original repository.

**Please ensure your code adheres to the project's coding standards and passes all tests.**


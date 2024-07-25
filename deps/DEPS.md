# Dependencies

## Overview

This document outlines the dependencies and setup process for the `nex-compiler` project, including LLVM, MLIR, and MLINR. It provides details on how to configure the build environment and include necessary libraries.

## Dependencies

### LLVM and MLIR

1. **LLVM**: The project requires LLVM version 18.1.8, which includes MLIR (Multi-Level Intermediate Representation).
2. **MLIR**: MLIR is part of LLVM and provides a flexible intermediate representation used in the `nex-compiler`.

### MLINR (Multi-Level Intermediate Nex Representation)

**NOTE: Versions 1.0.0 wont use MLINR as it is in the process of being developed**

**MLINR** is a new intermediate representation designed specifically for the `nex` compiler. It extends the capabilities of traditional intermediate representations by supporting multiple levels of abstraction and optimization stages. This enables more efficient compilation and better integration with various backends.

- **Repository**: [MLINR GitHub Repository](https://github.com/nex-lang/mlinr)
- **Features**:
  - Multi-level representation of NEX code
  - Flexibility for various optimization levels
  - Integration with LLVM for lower-level code generation

### Setup Process

1. **Download and Build LLVM with MLIR**

   The `check_llvm_build` function in `CMakeLists.txt` handles the setup of LLVM and MLIR. Here’s how it works:

   - **Check for Existing Build**: The script checks if `libLLVMCore.a` exists in the build directory to determine if LLVM and MLIR have already been built.

   - **Download LLVM Source**: If the source tarball is not present, it downloads it from [LLVM's release page](https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/llvm-project-18.1.8.src.tar.xz).

   - **Extract Source**: The tarball is extracted to the specified directory.

   - **Configure Build**: The build is configured with CMake, enabling MLIR and targeting X86.

   - **Build and Install**: LLVM and MLIR are built and installed into the specified directory.

   **Important Directories:**
   - Source Directory: `${DEPS_DIR}/llvm-project-18.1.8.src`
   - Build Directory: `${LLVM_SRC_DIR}/build`
   - Install Directory: `${LLVM_SRC_DIR}/install`

   **Build Commands:**
   ```bash
   cmake -S ${LLVM_SRC_DIR}/llvm -B ${LLVM_BUILD_DIR} -DLLVM_ENABLE_PROJECTS=mlir -DLLVM_TARGETS_TO_BUILD=X86 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${LLVM_INSTALL_DIR}
   cd ${LLVM_BUILD_DIR} && make
   ```

2. **Include Directories**

   After building LLVM and MLIR, ensure the following directories are included in the build:
   - `${LLVM_BUILD_DIR}/include`
   - `${LLVM_INSTALL_DIR}/lib/cmake/llvm`
   - `${LLVM_INSTALL_DIR}/lib/cmake/mlir`

   These directories contain the necessary headers and libraries for compiling the project.

3. **Integrate MLINR**

   To integrate MLINR into your build process, ensure you include its headers and libraries as needed. You might also need to configure additional CMake options or scripts depending on how MLINR is set up in your project.

### Criterion Testing Framework

1. **Test Libraries**

   The project optionally builds tests using the Criterion testing framework. If `BUILD_TESTS` is enabled, the setup will include downloading and configuring Criterion.

   - **Download Criterion**: The script checks if Criterion is available. If not, it downloads and extracts the appropriate version:
     - For Linux: `https://github.com/Snaipe/Criterion/releases/download/v2.3.3/criterion-v2.3.3-linux-x86_64.tar.bz2`
     - For Windows: Uses PowerShell to download and extract the zip file.

   **Download Commands:**
   ```bash
   curl -o deps/criterion.zip -LJO https://github.com/Snaipe/Criterion/releases/download/v2.3.3/criterion-v2.3.3-linux-x86_64.tar.bz2
   tar -xjf deps/criterion.zip -C deps
   rm deps/criterion.zip
   ```

   **Include Directories:**
   - `${CMAKE_CURRENT_SOURCE_DIR}/deps/criterion-v2.3.3/include`

   **Link Library:**
   - Criterion library is found and linked with the tests.

## Build Instructions

1. **Set Up Dependencies**

   Run the `check_llvm_build` function to ensure LLVM and MLIR are built and available.

2. **Configure and Build**

   Configure the project with CMake:
   ```bash
   mkdir build
   cd build
   cmake ..
   ```

   Build the project:
   ```bash
   make
   ```

3. **Run Tests (Optional)**

   To build and run tests, enable the `BUILD_TESTS` option:
   ```bash
   cmake -DBUILD_TESTS=ON ..
   make test
   ```

## Additional Notes

- Ensure all paths and dependencies are correctly set as described.
- For more information about MLINR, refer to its [GitHub repository](https://github.com/nex-lang/mlinr).
- For any issues or additional help, refer to the project’s documentation or open an issue on the [nex-lang GitHub repository](https://github.com/nex-lang/nex-compiler).

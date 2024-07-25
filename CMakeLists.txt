cmake_minimum_required(VERSION 3.20)
project(nex-compiler)

set(DEPS_DIR "${CMAKE_SOURCE_DIR}/deps")
set(LLVM_SRC_TAR "llvm-project-18.1.8.src.tar.xz")
set(LLVM_SRC_DIR "${DEPS_DIR}/llvm-project-18.1.8.src")
set(LLVM_BUILD_DIR "${LLVM_SRC_DIR}/build")
set(LLVM_INSTALL_DIR "${LLVM_SRC_DIR}/install")

function(check_llvm_build)
    if (NOT EXISTS "${LLVM_BUILD_DIR}/lib/libLLVMCore.a")
        message(STATUS "LLVM-MLIR not found, starting build")
        file(MAKE_DIRECTORY ${LLVM_BUILD_DIR})

        if (NOT EXISTS "${DEPS_DIR}/${LLVM_SRC_TAR}")
            message(STATUS "Downloading LLVM-SRC from https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/llvm-project-18.1.8.src.tar.xz")
            execute_process(
                COMMAND curl -L -o ${DEPS_DIR}/${LLVM_SRC_TAR} https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/llvm-project-18.1.8.src.tar.xz
                RESULT_VARIABLE result
                ERROR_VARIABLE error
                OUTPUT_QUIET
            )
        endif()

        if (NOT EXISTS "${LLVM_SRC_DIR}")
            message(STATUS "Extracting LLVM-MLIR from ${LLVM_SRC_TAR}")
            execute_process(
                COMMAND tar -xJf ${DEPS_DIR}/${LLVM_SRC_TAR} -C ${DEPS_DIR}
            )
        endif()

        message(STATUS "Configuring LLVM-MLIR build...")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -S ${LLVM_SRC_DIR}/llvm -B ${LLVM_BUILD_DIR}
                    -DLLVM_ENABLE_PROJECTS=mlir
                    -DLLVM_TARGETS_TO_BUILD=X86
                    -DCMAKE_BUILD_TYPE=Release
                    -DCMAKE_INSTALL_PREFIX=${LLVM_INSTALL_DIR}
        )

        message(STATUS "Building and installing LLVM-MLIR...")
        execute_process(
            COMMAND cd ${LLVM_BUILD_DIR}/lib/IR && make
        )

        message(STATUS "LLVM-MLIR has been successfully built!")
    else()
        message(STATUS "LLVM-MLIR found!")
    endif()
endfunction()

check_llvm_build()

cmake_policy(SET CMP0077 NEW)

include_directories(${LLVM_INCLUDE_DIRS} ${MLIR_INCLUDE_DIRS})
include_directories(${LLVM_BUILD_DIR}/include)

link_directories(${LLVM_INSTALL_DIR}/lib)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

# Define sources and headers
set(LIB_SOURCES
    include/tmp/alphadev.c
    src/io.c
    src/symtbl.c
    src/lexer.c
    src/ast.c
    src/parser.c
    src/sao.c
    src/imc.c
    src/gen.c
    src/gen/arth.c
    src/gen/class.c
    src/gen/control.c
    src/gen/enum.c
    src/gen/err.c
    src/gen/func.c
    src/gen/imports.c
    src/gen/mem.c
    src/gen/stack.c
    src/gen/struct.c
)

set(EXE_SOURCES
    src/main.c
)

set(HEADERS
    include/tmp/alphadev.h
    include/utils/bit128.h
    include/p_info.h
    include/io.h
    include/token.h
    include/symtbl.h
    include/lexer.h
    include/ast.h
    include/parser.h
    include/sao.h
    include/imc.h
    include/gen.h
    include/gen/arth.h
    include/gen/class.h
    include/gen/control.h
    include/gen/enum.h
    include/gen/err.h
    include/gen/func.h
    include/gen/import.h
    include/gen/mem.h
    include/gen/stack.h
    include/gen/struct.h
)

include_directories(include)

add_library(nex_library OBJECT ${LIB_SOURCES} ${HEADERS})
add_executable(nex $<TARGET_OBJECTS:nex_library> ${EXE_SOURCES})

# Build tests if requested
option(BUILD_TESTS "Build tests" OFF)

if(BUILD_TESTS)
    set(t_SOURCES 
        tests/lexer_test.c
        tests/symtbl_test.c
    )
        
    message("nex-compiler:cmake $> [Building tests]")

    if(NOT TARGET test)
        if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/deps/criterion-v2.3.3)
            message("nex-compiler:cmake $> [Downloading Criterion]")

            if(WIN32)
                execute_process(COMMAND powershell -Command "${CMAKE_COMMAND} -E make_directory deps; Invoke-WebRequest -Uri 'https://github.com/Snaipe/Criterion/releases/download/v2.3.3/criterion-v2.3.3-win64-mingw.zip' -OutFile 'deps/criterion.zip'; Expand-Archive -Path 'deps/criterion.zip' -DestinationPath 'deps/'; Remove-Item -Path 'deps/criterion.zip' -Force")
            else()
                execute_process(COMMAND mkdir deps/)
                execute_process(COMMAND curl -o deps/criterion.zip -LJO https://github.com/Snaipe/Criterion/releases/download/v2.3.3/criterion-v2.3.3-linux-x86_64.tar.bz2)
                execute_process(COMMAND tar -xjf deps/criterion.zip -C deps)
                execute_process(COMMAND rm deps/criterion.zip)
            endif()
        endif()

        find_library(CRITERION_LIB criterion HINTS ${CMAKE_CURRENT_SOURCE_DIR}/deps/criterion-v2.3.3/lib)
        
        add_executable(test $<TARGET_OBJECTS:nex_library> ${t_SOURCES})
        target_include_directories(test PRIVATE include)
        target_include_directories(test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/deps/criterion-v2.3.3/include)
        target_link_libraries(test PRIVATE ${CRITERION_LIB})
    else()
        message("nex-compiler:cmake $> [Test target already exists]")
    endif()
endif()

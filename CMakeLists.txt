cmake_minimum_required(VERSION 3.5)
project(nex-compiler)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

set(LIB_SOURCES
    src/io.c
    src/symtbl.c
    src/lexer.c
    src/ast.c
    src/parser.c
)

set(EXE_SOURCES
    src/main.c
)

set(HEADERS
    include/p_info.h
    include/io.h
    include/token.h
    include/symtbl.h
    include/lexer.h
    include/ast.h
    include/parser.h
)

include_directories(include)

add_library(nex_library OBJECT ${LIB_SOURCES} ${HEADERS})
add_executable(nex $<TARGET_OBJECTS:nex_library> ${EXE_SOURCES})

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
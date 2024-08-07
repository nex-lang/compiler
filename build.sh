#!/bin/bash

GREEN=$(tput setaf 2)
LIME_YELLOW=$(tput setaf 190)
MAGENTA=$(tput setaf 5)
BRIGHT=$(tput bold)
NORMAL=$(tput sgr0)
UNDERLINE=$(tput smul)

CURRENT_VERSION=$(cat version.txt)
BUILD_TESTS=OFF
EXAMPLE=""

print_help() {
    printf "${MAGENTA}${BRIGHT}[BUILD.SH]${NORMAL} Usage: ./build.sh ${LIME_YELLOW}(-D | -R) [OPTIONS]${NORMAL} \n${UNDERLINE}Options:${NORMAL}\n"
    printf "  ${BRIGHT}-H, --help${NORMAL}           Display this message [mitigates all options]\n"
    printf "  ${BRIGHT}-T, --test${NORMAL}           Equivalent to -DBUILD_TESTS=ON [see CMakeLists.txt]\n"
    printf "  ${BRIGHT}-D, --debug${NORMAL}          Equivalent to -DCMAKE_BUILD_TYPE=Debug [see CMake Documentation]\n"
    printf "  ${BRIGHT}-R, --release${NORMAL}        Equivalent to -DCMAKE_BUILD_TYPE=Release [see CMake Documentation]\n"
    printf "  ${BRIGHT}-C, --clean${NORMAL}          Force clearing build directory\n"
    printf "  ${BRIGHT}-P, --profile <example>${NORMAL} Profile the given example\n"
}

# Parse arguments

if [ "$#" -lt 1 ]; then
    printf "${MAGENTA}${BRIGHT}[BUILD.SH]${NORMAL} INSUFFICIENT ARGUMENTS PASSED (build.sh)\n"
    printf "${MAGENTA}${BRIGHT}[BUILD.SH]${NORMAL} USE --help or -H flag to find a usage guide\n"
    exit 1
fi

while (( "$#" )); do
    case "$1" in
        -H|--help)
            print_help
            exit 0
            ;;
        -C|--clean)
            printf "${MAGENTA}${BRIGHT}[BUILD.SH]${NORMAL} FORCE CLEARING BUILD DIRECTORY\n"
            rm -rf build/
            exit 0
            ;;
        -T|--test)
            BUILD_TESTS=ON
            shift
            ;;
        -P|--profile)
            EXAMPLE="$2"
            shift 2
            ;;
        -D|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -R|--release)
            BUILD_TYPE="Release"
            shift
            ;;
        -*|--*)
            printf "${MAGENTA}${BRIGHT}[BUILD.SH]${NORMAL} INVALID ARGUMENT: $1\n"
            printf "${MAGENTA}${BRIGHT}[BUILD.SH]${NORMAL} USE --help or -H flag to find a usage guide\n"
            exit 1
            ;;
        *)
            shift
            ;;
    esac
done

if [ "$BUILD_TYPE" == "Debug" ]; then
    printf "${MAGENTA}${BRIGHT}[BUILD.SH]${NORMAL} ${GREEN}LATEST VERIFIED VERSION ${CURRENT_VERSION}${NORMAL}: BUILDING IN DEBUG MODE\n"
    cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=${BUILD_TESTS}
    cd build/ && make
elif [ "$BUILD_TYPE" == "Release" ]; then
    printf "${MAGENTA}${BRIGHT}[BUILD.SH]${NORMAL} LATEST RELEASE VERSION ${CURRENT_VERSION}: BUILDING IN RELEASE MODE\n"
    cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=${BUILD_TESTS}
    cd build/ && make
fi

if [ -n "$EXAMPLE" ]; then
    valgrind --log-file=logs.txt --time-stamp=yes --leak-check=full --track-origins=yes --show-leak-kinds=all ./nex ../examples/$EXAMPLE.nex -o $EXAMPLE
fi
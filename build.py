import argparse
import os
import subprocess
import sys
import platform

GREEN = '\033[32m'
LIME_YELLOW = '\033[93m'
MAGENTA = '\033[35m'
BRIGHT = '\033[1m'
NORMAL = '\033[0m'
UNDERLINE = '\033[4m'

if platform.system() == "Windows":
    os.system('')

def print_colored(color, text):
    print(f"{color}{text}{NORMAL}")

def run_command(command):
    try:
        subprocess.run(command, check=True, shell=True)
    except subprocess.CalledProcessError as e:
        print(f"Command '{' '.join(command)}' failed with exit code {e.returncode}")
        sys.exit(e.returncode)

def main():
    parser = argparse.ArgumentParser(description="Build script", add_help=False)
    parser.add_argument("-H", "--help", action="store_true", help="Display this message [mitigates all options]")
    parser.add_argument("-T", "--test", action="store_true", help="Equivalent to -DBUILD_TESTS=ON [see CMakeLists.txt]")
    parser.add_argument("-D", "--debug", action="store_true", help="Equivalent to -DCMAKE_BUILD_TYPE=Debug [see CMake Documentation]")
    parser.add_argument("-R", "--release", action="store_true", help="Equivalent to -DCMAKE_BUILD_TYPE=Release [see CMake Documentation]")
    parser.add_argument("-C", "--clean", action="store_true", help="FORCE CLEARING BUILD DIRECTORY")
    
    args = parser.parse_args()

    try:
        with open("version.txt", "r") as file:
            current_version = file.read().strip()
    except FileNotFoundError:
        print_colored(MAGENTA, "[BUILD.PY] version.txt not found.")
        sys.exit(1)

    build_tests = "OFF"
    if args.test:
        build_tests = "ON"

    if args.help:
        print_colored(MAGENTA, f"{BRIGHT}[BUILD.PY]{NORMAL} Usage: python build.py {LIME_YELLOW}[OPTIONS]{NORMAL}")
        print(f"{UNDERLINE}Options:{NORMAL}")
        print(f"  {BRIGHT}-H, --help{NORMAL}           Display this message [mitigates all options]")
        print(f"  {BRIGHT}-T, --test{NORMAL}           Equivalent to -DBUILD_TESTS=ON [see CMakeLists.txt]")
        print(f"  {BRIGHT}-D, --debug{NORMAL}          Equivalent to -DCMAKE_BUILD_TYPE=Debug [see CMake Documentation]")
        print(f"  {BRIGHT}-R, --release{NORMAL}        Equivalent to -DCMAKE_BUILD_TYPE=Release [see CMake Documentation]")
        print(f"  {BRIGHT}-C, --clean{NORMAL}          FORCE CLEARING BUILD DIRECTORY")
        sys.exit(0)

    if args.clean:
        print_colored(MAGENTA, f"{BRIGHT}[BUILD.PY]{NORMAL} FORCE CLEARING BUILD DIRECTORY")
        if platform.system() == "Windows":
            run_command(["rmdir", "/S", "/Q", "build"])
        else:
            run_command(["rm", "-rf", "build/"])
        sys.exit(0)

    if not (args.debug or args.release):
        print_colored(MAGENTA, f"{BRIGHT}[BUILD.PY]{NORMAL} INSUFFICIENT ARGUMENTS PASSED (build.py)")
        print_colored(MAGENTA, f"{BRIGHT}[BUILD.PY]{NORMAL} USE --help or -H flag to find a usage guide")
        sys.exit(1)

    cmake_command = ["cmake", "-B", "build"]
    if args.debug:
        cmake_command.extend(["-DCMAKE_BUILD_TYPE=Debug", f"-DBUILD_TESTS={build_tests}"])
        print_colored(MAGENTA, f"{BRIGHT}[BUILD.PY]{NORMAL} {GREEN}LATEST VERIFIED VERSION {current_version}{NORMAL}: BUILDING IN DEBUG MODE")
    elif args.release:
        cmake_command.extend(["-DCMAKE_BUILD_TYPE=Release", f"-DBUILD_TESTS={build_tests}"])
        print_colored(MAGENTA, f"{BRIGHT}[BUILD.PY]{NORMAL} LATEST RELEASE VERSION {current_version}: BUILDING IN RELEASE MODE")

    run_command(cmake_command)
    os.chdir("build")
    run_command(["make" if platform.system() != "Windows" else "cmake --build ."])

if __name__ == "__main__":
    main()
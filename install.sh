#!/bin/bash

GREEN=$(tput setaf 2)
MAGENTA=$(tput setaf 5)
BRIGHT=$(tput bold)
NORMAL=$(tput sgr0)

INSTALL_DIR="/usr/local/bin"

usage() {
    printf "${MAGENTA}${BRIGHT}[INSTALL.SH]${NORMAL} Usage: ./install.sh [ --vsce ]\n"
    exit 1
}

if [ "$1" == "--vsce" ]; then
    printf "${MAGENTA}${BRIGHT}[INSTALL.SH]${NORMAL} Installing VS Code extension"
    
    curl -L -o /tmp/nex-0.0.1.vsix "https://github.com/nex-lang/vsc-extension/releases/download/v.1.0.0/nex-1.0.0.vsix"
    code --install-extension /tmp/nex-1.0.0.vsix

    printf "${MAGENTA}${BRIGHT}[INSTALL.SH]${NORMAL} VS Code extension installation successful!"
fi

if [ ! -d "build" ]; then
    printf "${MAGENTA}${BRIGHT}[INSTALL.SH]${NORMAL} Executable 'nex' not found in build directory. Running build.sh --release\n"
    ./build.sh --release

    if [ ! -f "build/nex" ]; then
        printf "${MAGENTA}${BRIGHT}[INSTALL.SH]${NORMAL} Build failed. Executable 'nex' not found.\n"
        exit 1
    fi
fi

if [ ! -f "build/nex" ]; then
    printf "${MAGENTA}${BRIGHT}[INSTALL.SH]${NORMAL} Executable 'nex' not found in build directory. Running build.sh --release\n"
    ./build.sh --release

    if [ ! -f "build/nex" ]; then
        printf "${MAGENTA}${BRIGHT}[INSTALL.SH]${NORMAL} Build failed. Executable 'nex' not found.\n"
        exit 1
    fi
fi

printf "${MAGENTA}${BRIGHT}[INSTALL.SH]${NORMAL} Installing 'nex' to ${INSTALL_DIR}\n"
sudo cp build/nex ${INSTALL_DIR}

if [ $? -eq 0 ]; then
    printf "${GREEN}${BRIGHT}[INSTALL.SH]${NORMAL} Installation successful!\n"
else
    printf "${MAGENTA}${BRIGHT}[INSTALL.SH]${NORMAL} Installation failed. Please check your permissions.\n"
    exit 1
fi

if [[ ":$PATH:" != *":${INSTALL_DIR}:"* ]]; then
    printf "${MAGENTA}${BRIGHT}[INSTALL.SH]${NORMAL} Adding ${INSTALL_DIR} to PATH\n"
    echo "export PATH=\$PATH:${INSTALL_DIR}" >> ~/.bashrc
    source ~/.bashrc
fi

printf "${GREEN}${BRIGHT}[INSTALL.SH]${NORMAL} 'nex' is now installed and available in your PATH.\n"

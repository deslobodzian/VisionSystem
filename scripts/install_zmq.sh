#!/bin/bash
set -e

ZEROMQ_VERSION="4.3.4"
DOWNLOAD_URL="https://github.com/zeromq/libzmq/archive/refs/tags/v${ZEROMQ_VERSION}.tar.gz"

TEMP_DIR=$(mktemp -d)
trap "rm -rf ${TEMP_DIR}" EXIT

cd ${TEMP_DIR}

echo "Downloading ZeroMQ v${ZEROMQ_VERSION}..."
curl -L -o ${ZEROMQ_VERSION}.tar.gz ${DOWNLOAD_URL}

echo "Extracting..."
tar -xzf ${ZEROMQ_VERSION}.tar.gz

cd libzmq-${ZEROMQ_VERSION}

install_dependencies() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        apt-get update
        apt-get install -y build-essential autoconf libtool pkg-config
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        brew update
        brew install autoconf libtool pkg-config
    else
        echo "Unknown OS: $OSTYPE"
        exit 1
    fi
}

echo "Installing dependencies..."
install_dependencies

echo "Building ZeroMQ..."
mkdir build
cd build
cmake ..
make -j$(sysctl -n hw.logicalcpu || nproc)
make install

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    ldconfig
fi

echo "ZeroMQ ${ZEROMQ_VERSION} installed successfully."

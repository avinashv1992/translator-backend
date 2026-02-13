#!/bin/bash

set -e

echo "Installing dependencies..."
apt-get update
apt-get install -y libcurl4-openssl-dev

echo "Building server..."
g++ src/main.cpp \
-Iinclude \
-lcurl \
-pthread \
-o server

echo "Build complete"

#!/bin/bash
set -e

echo "Updating system..."
apt-get update

echo "Installing compiler and dependencies..."
apt-get install -y build-essential libcurl4-openssl-dev

echo "Building server..."
g++ src/*.cpp \
-Iinclude \
-lcurl \
-pthread \
-o server

echo "Build complete"

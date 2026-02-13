#!/bin/bash
set -e

echo "Updating system..."
apt-get update

echo "Installing compiler and dependencies..."
apt-get install -y build-essential libcurl4-openssl-dev

echo "Finding source files..."

FILES=$(find . -name "*.cpp")

echo "Found files:"
echo "$FILES"

if [ -z "$FILES" ]; then
  echo "ERROR: No .cpp files found!"
  exit 1
fi

echo "Building server..."
g++ $FILES \
-Iinclude \
-lcurl \
-pthread \
-o server

echo "Build complete"

#!/bin/bash
set -e

echo "Installing dependencies..."
apt-get update
apt-get install -y build-essential libcurl4-openssl-dev

echo "Finding source files..."

FILES=$(find backend/src -name "*.cpp")

echo "Found files:"
echo "$FILES"

if [ -z "$FILES" ]; then
  echo "ERROR: No .cpp files found!"
  exit 1
fi

echo "Building server..."

g++ $FILES \
-Ibackend/include \
-Ibackend/include/crow \
-lcurl \
-pthread \
-o server

echo "Build finished successfully"

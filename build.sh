#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

if [ ! -d "build" ]; then 
    echo "Creating build ....."
    # Run the build commands
    cmake -B build .
fi

cd build/
make install
clear

# Run the executable
./run
#!/bin/bash
# Build script for Pebble Vocab Learner
# Handles automatic renaming of .pbw file

echo "Building Pebble Vocab Learner..."
pebble build "$@"

if [ $? -eq 0 ]; then
    # Rename the .pbw file to use underscores
    if [ -f "build/myfirstproject.pbw" ]; then
        mv build/myfirstproject.pbw build/pebble_vocab_learner.pbw
        echo "✓ Renamed to: build/pebble_vocab_learner.pbw"
    fi
    echo "✓ Build completed successfully"
else
    echo "✗ Build failed"
    exit 1
fi

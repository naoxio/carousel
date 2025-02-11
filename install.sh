#!/usr/bin/env bash


# Create ~/bin if it doesn't exist
mkdir -p ~/bin

# Remove old symlink if it exists
rm -f ~/bin/carousel

# Create new symlink
cp "$(pwd)/build/carousel" ~/bin/carousel

echo "Installed carousel to ~/bin/"

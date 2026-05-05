#!/bin/bash

# GitHub push script for OpenDOS/plDOS
# Usage: ./push.sh major|minor

set -e

# Set git user configuration
git config user.name "codebunny100"
git config user.email "es.wendland@gmail.com"

# Check and set remote origin if not exists
if ! git remote get-url origin > /dev/null 2>&1; then
    echo "Adding remote origin: https://github.com/Poniek-Labs/plDOS.git"
    git remote add origin https://github.com/Poniek-Labs/plDOS.git
fi

# Check if VERSION file exists, create with initial version if not
if [ ! -f VERSION ]; then
    echo "1.0.0" > VERSION
    echo "Created VERSION file with initial version 1.0.0"
fi

# Read current version
version=$(cat VERSION)
echo "Current version: $version"

# Parse version (assume MAJOR.MINOR.PATCH format)
IFS='.' read -r major minor patch <<< "$version"

# Validate input
if [ $# -ne 1 ]; then
    echo "Usage: $0 major|minor"
    echo "  major: Increment major version (resets minor and patch to 0)"
    echo "  minor: Increment minor version (resets patch to 0)"
    exit 1
fi

# Update version based on argument
case "$1" in
    major)
        major=$((major + 1))
        minor=0
        patch=0
        ;;
    minor)
        minor=$((minor + 1))
        patch=0
        ;;
    *)
        echo "Invalid option: $1"
        echo "Usage: $0 major|minor"
        exit 1
        ;;
esac

# Create new version string
new_version="$major.$minor.$patch"
echo "New version: $new_version"

# Write new version to file
echo "$new_version" > VERSION

# Git operations
echo "Adding all changes to git (excluding docs and desktop)..."
git add --all -- :!docs :!progs/desktop

echo "Committing with message 'Version $new_version'..."
git commit -m "Version $new_version"

echo "Pushing to origin main..."
git push origin main

echo "Successfully pushed version $new_version to GitHub!"
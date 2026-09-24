#!/bin/bash

set -e

if [[ -z "$1" ]]; then
    echo "Error: commit message is required."
    echo "Usage: $0 <commit-message>"
    exit 1
fi

git status
git add .
git commit -m "$1"
git push


#!/bin/bash

set -e

current_branch=$(git rev-parse --abbrev-ref HEAD)
if [[ "$current_branch" != "dev" ]]; then
    echo "Error: current branch is '$current_branch', but only 'dev' branch is allowed."
    echo "Please switch to dev branch first: git checkout dev"
    exit 1
fi

if [[ $# -eq 0 ]]; then
    echo "Error: commit message is required."
    echo "Usage: $0 <commit-message>"
    exit 1
fi

printf "\n"
echo "commit -m " "\"$*\""
printf "\n"

git status
git add .
git commit -m "$*"
git push


#!/bin/bash

#set -e
#git checkout main
#git pull origin main
#git merge dev
#git push origin main
#git checkout dev

#########################################
set -euo pipefail

# 必须从 dev 分支执行
if [[ "$(git branch --show-current)" != "dev" ]]; then
    echo "Error: current branch must be dev"
    exit 1
fi

git fetch origin

# 确认远程 main 是 dev 的祖先，避免覆盖 main 上的独立提交
if ! git merge-base --is-ancestor origin/main dev; then
    echo "Error: origin/main is not an ancestor of dev."
    echo "Please merge origin/main into dev before publishing."
    exit 1
fi

git push origin dev:main

echo "Successfully pushed dev to origin/main."
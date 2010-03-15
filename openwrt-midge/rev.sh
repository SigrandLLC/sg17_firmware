#!/bin/sh

LANG=C

# show revision number, like in subversion
# 9 commits were lost, so add them
revision=$(( $(git rev-list HEAD | wc -l) + 9 ))
#echo "revision: $revision"

branch=`git branch --no-color | egrep '^\*' | cut '-d ' -f2`
#branch=1.2
#echo "branch: $branch"

echo $branch-$revision


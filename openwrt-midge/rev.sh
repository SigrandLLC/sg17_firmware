#!/bin/sh

# show revision number, like in subversion
echo $(( $(git rev-list HEAD|wc -l) + 9 ))

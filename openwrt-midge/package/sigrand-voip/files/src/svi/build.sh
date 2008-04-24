#!/bin/bash

# $1 - path to mipsel bin dir

$1mipsel-linux-uclibc-gcc -I./vinetic/include/ -I./tapi/include/ -I./sgatab/ svi.c -o svi


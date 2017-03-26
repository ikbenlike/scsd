#!/bin/bash

cc="gcc"
cargs="-Wall -g --std=c99"


bash -c "$cc $cargs source/server/main.c source/server/http_server_utils.c -o builds/main.out"

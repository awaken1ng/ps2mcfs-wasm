#!/bin/env sh

emcmake cmake . -B build
cmake --build ./build --target mcfs

#!/bin/bash

mkdir -p build

cd build || exit

cmake ..

read -p "Press any key to continue . . ."
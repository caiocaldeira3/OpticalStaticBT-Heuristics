#!/bin/bash

make main

for file in input/$1-*; do
    make run ARGS="$file shuffle"

done

for file in input/$1-*; do
    make run ARGS="$file static"

done
#!/bin/bash


make main

for file in input/no_locality-*; do
    make run ARGS="$file shuffle"

done

for file in input/no_locality-*; do
    make run ARGS="$file static"

done
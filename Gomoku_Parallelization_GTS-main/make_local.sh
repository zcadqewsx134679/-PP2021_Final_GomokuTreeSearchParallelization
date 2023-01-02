#!/bin/bash

mkdir test
cd test && cmake .. && sudo make install
cd .. && rm -rf test

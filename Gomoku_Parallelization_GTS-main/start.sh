#!/bin/bash
sudo docker run --cpuset-cpus="0-7" --rm -p 8000:8000 -e SERVER_URI=http://$1:8000 gomoku


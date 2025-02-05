#!/bin/bash

for file in ./*.bmp
do
    magick $file -compress none -depth 8 -type TrueColor -flip $file
done

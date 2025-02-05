#!/bin/bash

for file in ./*.jpg
do
    magick $file -compress none -depth 8 -type TrueColor $file
done

for file in ./*.png
do
    magick $file -compress none -depth 8 -type TrueColor $file
done

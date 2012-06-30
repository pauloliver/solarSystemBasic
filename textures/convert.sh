#!/bin/bash

for i in *.jpg
do
convert $i -resize 512x256 ${i%%.*}.bmp
echo $i processed
done

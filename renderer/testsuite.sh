#!/bin/sh
DATE=`date +'%Y-%m-%d %H:%M:%S'`
STYLE=styles/osmorg/osmorg.xml

./render.py --style $STYLE --date $DATE --zoom 1 --file 01
./render.py --style $STYLE --date $DATE --zoom 2 --file 02
./render.py --style $STYLE --date $DATE --bbox -100,-10,100,70 --zoom 3 --file 03
./render.py --style $STYLE --date $DATE --bbox -17.14,56.37,27.61,35.34 --zoom 4 --file 04
./render.py --style $STYLE --date $DATE --bbox -17.14,56.37,27.61,35.34 --zoom 5 --file 05
./render.py --style $STYLE --date $DATE --bbox 5.56,55.04,15.35,47.15 --zoom 6 --file 06
./render.py --style $STYLE --date $DATE --bbox 5.56,55.04,15.35,47.15 --zoom 7 --file 07
./render.py --style $STYLE --date $DATE --bbox 7.24,51.04,10.21,49.5 --zoom 8 --file 08
./render.py --style $STYLE --date $DATE --bbox 7.24,51.04,10.21,49.5 --zoom 9 --file 09
./render.py --style $STYLE --date $DATE --bbox 7.552,49.525,8.834,50.028 --zoom 10 --file 10
./render.py --style $STYLE --date $DATE --bbox 7.971,49.651,8.415,49.903 --zoom 11 --file 11
./render.py --style $STYLE --date $DATE --bbox 8.0818,49.7141,8.3042,49.8398 --zoom 12 --file 12
./render.py --style $STYLE --date $DATE --bbox 8.1374,49.7456,8.2486,49.8084 --zoom 13 --file 13
./render.py --style $STYLE --date $DATE --bbox 8.1508,49.7655,8.2308,49.7969 --zoom 14 --file 14
./render.py --style $STYLE --date $DATE --bbox 8.1769,49.77334,8.2047,49.78906 --zoom 15 --file 15
./render.py --style $STYLE --date $DATE --bbox 8.18389,49.77981,8.1978,49.78766 --zoom 16 --file 16
./render.py --style $STYLE --date $DATE --bbox 8.18898,49.78206,8.19593,49.78598 --zoom 17 --file 17
./render.py --style $STYLE --date $DATE --bbox 8.191165,49.783138,8.194641,49.785102 --zoom 18 --file 18

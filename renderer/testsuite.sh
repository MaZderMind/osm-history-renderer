#!/bin/sh
DATE=`date +'%Y-%m-%d %H:%M:%S'`
STYLE=styles/osmorg/osmorg.xml

./render.py --style $STYLE --date $DATE --bbox -180,-85,180,85 --zoom 1 --file 01
./render.py --style $STYLE --date $DATE --bbox -180,-70,202,85 --zoom 2 --file 02
./render.py --style $STYLE --date $DATE --bbox -88.5,-20.5,104.9,79.4 --zoom 3 --file 03
./render.py --style $STYLE --date $DATE --bbox -40.1,18,56.5,69.1 --zoom 4 --file 04
./render.py --style $STYLE --date $DATE --bbox -16,35.4,32.4,60.9 --zoom 5 --file 05
./render.py --style $STYLE --date $DATE --bbox -3.89,43.02,20.28,55.72 --zoom 6 --file 06
./render.py --style $STYLE --date $DATE --bbox 2.15,46.51,14.24,52.85 --zoom 7 --file 07
./render.py --style $STYLE --date $DATE --bbox 5.17,48.17,11.21,51.34 --zoom 8 --file 08
./render.py --style $STYLE --date $DATE --bbox 6.682,48.985,9.703,50.57 --zoom 9 --file 09
./render.py --style $STYLE --date $DATE --bbox 7.438,49.386,8.948,50.179 --zoom 10 --file 10
./render.py --style $STYLE --date $DATE --bbox 7.815,49.586,8.571,49.982 --zoom 11 --file 11
./render.py --style $STYLE --date $DATE --bbox 8.004,49.685,8.3817,49.8832 --zoom 12 --file 12
./render.py --style $STYLE --date $DATE --bbox 8.0984,49.7347,8.2873,49.8337 --zoom 13 --file 13
./render.py --style $STYLE --date $DATE --bbox 8.1457,49.7594,8.2401,49.809 --zoom 14 --file 14
./render.py --style $STYLE --date $DATE --bbox 8.16926,49.77183,8.21646,49.7966 --zoom 15 --file 15
./render.py --style $STYLE --date $DATE --bbox 8.18106,49.77803,8.20466,49.79041 --zoom 16 --file 16
./render.py --style $STYLE --date $DATE --bbox 8.18696,49.78112,8.19876,49.78732 --zoom 17 --file 17
./render.py --style $STYLE --date $DATE --bbox 8.18991,49.782672,8.19581,49.785769 --zoom 18 --file 18

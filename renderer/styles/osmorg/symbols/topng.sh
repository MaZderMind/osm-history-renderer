#!/bin/bash

TYPES=(             'accommodation' 'amenity' 'barrier' 'education' 'food'    'health'  'landuse' 'money'   'place_of_worship' 'poi'     'power'    'shopping' 'sport'   'tourist' 'transport' 'water')

for (( i = 0 ; i < ${#TYPES[@]} ; i++ )) do

  if  [ "$1" == "" -o "$1" == "${TYPES[i]}" ]; then

    echo -n "${TYPES[i]}"

    for FILE in color/${TYPES[i]}/*.svg; do

      for SIZE in 16 20 24; do

        BASENAME=$(basename ${FILE})
        BASENAME=png/${SIZE}/${TYPES[i]}/${BASENAME%.*}

        mkdir -p png/${SIZE}/${TYPES[i]}

        echo -n "."
        rsvg --width=${SIZE} ${FILE} ${BASENAME}.png

      done

    done
    echo ""

  fi

done

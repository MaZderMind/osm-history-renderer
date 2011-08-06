#!/bin/bash

TYPES=(             'accommodation' 'amenity' 'barrier' 'education' 'food'    'health'  'landuse' 'money'   'place_of_worship' 'poi'     'power'    'shopping' 'sport'   'tourist' 'transport' 'water')
FORGROUND_COLOURS=( '#0092DA'       '#734A08' '#666666' '#39AC39'   '#734A08' '#DA0092' '#999999' '#000000' '#000000'          '#000000'  '#8e7409'  '#AC39AC'  '#39AC39' '#734A08' '#0092DA'   '#0092DA')

OUTPUTFOLDER=color

for (( i = 0 ; i < ${#TYPES[@]} ; i++ )) do

  if  [ "$1" == "" -o "$1" == "${TYPES[i]}" ]; then

    echo -n "${TYPES[i]}"

    for FILE in mono/${TYPES[i]}/*.svg; do

      BASENAME=$(basename ${FILE})
      BASENAME=${OUTPUTFOLDER}/${TYPES[i]}/${BASENAME%.*}

      mkdir -p ${OUTPUTFOLDER}/${TYPES[i]}

      echo -n "."
      ./recolour.sh ${FILE} 'none' 'none' ${FORGROUND_COLOURS[i]} >${BASENAME}.p.svg
      ./recolour.sh ${FILE} ${FORGROUND_COLOURS[i]} ${FORGROUND_COLOURS[i]} '#ffffff' >${BASENAME}.n.svg

    done
    echo ""

  fi

done

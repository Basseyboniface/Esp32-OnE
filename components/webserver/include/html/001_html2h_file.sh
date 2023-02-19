#!/bin/bash

echo ".html to .h file converter"
for file in *.html
do 
	GZ_FILE="$file.gz"
	GZ_H_FILE=${GZ_FILE//[.]/_}.h
	
	echo ... $file "-> .gz -> .h"
	gzip -c $file > $GZ_FILE;
	xxd -i $GZ_FILE > $GZ_H_FILE;
	
	mv $GZ_H_FILE ../
	rm $GZ_FILE
done


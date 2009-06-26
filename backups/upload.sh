#!/bin/sh
set -e
if [ $# -eq 2 ]
then
#	echo "python googlecode_upload.py  -s \"$1\" -p cs143-grading-application -u juliausko -w bZ9Cc3yy6pg3 $2"
	python googlecode_upload.py  -s "$1" -p cs143-grading-application -u juliausko -w bZ9Cc3yy6pg3 $2
else
	echo "Two Parameters required: Summary and File Name"
fi

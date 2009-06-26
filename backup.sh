#!/bin/sh
set -e
NOW=$(date +"%b-%d-%y-%Hh-%Mm-%Ss")

if [ $# -lt 2 ]
then
	echo "Enter directory to compress: "
	read name
	echo "Enter upload summary: "
	read summary
else
	name=$1;
	summary=$2;
fi
	#remove slashes from directory name
	fname=$(echo $name | sed 's/\///g')
	echo "Backing up folder $fname"
	tarname="BACKUP-$fname-$NOW.tar"
	tar -cvzf backups/$tarname $name
	echo "Compression Successful."

#	chdir backups
#	python googlecode_upload.py  -s "$summary" -p cs143-grading-application -u juliausko -w bZ9Cc3yy6pg3 $tarname

	echo "Uploading to GoogleCode. Done."

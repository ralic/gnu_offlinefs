#!/bin/bash
#     Copyright (C) 2008 Francisco Jerez
#     This file is part of offlinefs.

#     offlinefs is free software: you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation, either version 3 of the License, or
#     (at your option) any later version.

#     offlinefs is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.

#     You should have received a copy of the GNU General Public License
#     along with offlinefs.  If not, see <http://www.gnu.org/licenses/>.

usage(){
    echo "usage: offimport_cd.sh -i <cdroot> -l <label> [-s <insertion script>] [-b <dbroot>] [-p <prefix>] [-f <config file>]"
    echo "Import a directory tree (<cdroot>) into the offlinefs with database at <dbroot>, prefixing each path with <prefix>. <insert script> will get called when trying to access any of the imported files."
    exit 1
}

dbroot="$HOME/.offlinefs" 
config="$HOME/.offlinefs/offlinefs.conf"
insert="/usr/local/etc/offlinefs/insert"
cdroot=""
label=""
prefix="/"

while getopts l:i:b:s: opt; do
    case $opt in
	l) label=$OPTARG;;
	i) cdroot=$OPTARG;;
	s) insert=$OPTARG;;
	d) dbroot=$OPTARG;;
	p) prefix=$OPTARG;;
    esac
done
[ $((OPTIND-1)) != $# ] && usage

[ -z $label ] && usage
[ -z $cdroot ] && usage
[ ${cdroot:0:1} == "/" ] || cdroot=`pwd`"/$cdroot"

echo "cdroot: $cdroot"
echo "label: $label"
echo "dbroot: $dbroot"
echo "insert script: $insert"
echo "prefix: $prefix"

echo "Adding medium to the database..."
fingerprint=`stat -f -c %a%b%c%d%f%i%l%s%S%t $cdroot`
offmedia --add insert --directory "$cdroot" --label "$label" --checkcmd "test $fingerprint = \`stat -f -c %a%b%c%d%f%i%l%s%S%t $cdroot\`" --insertscript $insert $dbroot ||exit
mid=`offmedia --list $dbroot |grep -a "#MEDIUM: " |sed "s/#MEDIUM: //" |tail -1 `

echo "Adding files to the database..."
find $cdroot -printf '%U %G %m %y %A@ %T@ %s %P //// %l\n' |awk 'NR!=1 {print $0}' |offimport -c -M $mid -b "$dbroot" -p "$prefix" -f  '%U %G %m %y %A@ %T@ %s %P //// %l'
echo "Done"

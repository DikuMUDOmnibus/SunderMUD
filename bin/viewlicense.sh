#!/bin/sh
DIALOG=${DIALOG=dialog}
tempfile=`tempfile 2>/dev/null` || tempfile=/tmp/test$$
trap "rm -f $tempfile" 0 1 2 5 15

cat << EOF > $tempfile
Welcome to SunderMud ][. You must view and agree to the applicable
license information before you may use this software. After reading
the license fully. If you do not agree to this license, you should
not compile or use this software in any way.


EOF

TEXT=../doc/LICENSE

cat $TEXT | expand >> $tempfile

$DIALOG --clear --no-shadow --title "License Information" --textbox "$tempfile" 0 0

case $? in
  0)
    echo "Ok";;
  255)
    exit 1;;
esac
 
 
 
 
 
 
 
 
 
 
 
 
 


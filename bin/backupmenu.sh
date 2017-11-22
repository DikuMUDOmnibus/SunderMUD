#!/bin/sh
DIALOG=${DIALOG=dialog}
tempfile=tempfile 2>/dev/null || tempfile=/tmp/test$$
trap "rm -f $tempfile" 0 1 2 5 15

$DIALOG --clear --no-shadow --title "SunderMud 2.0 Backup Control" \
--menu "Here you can make backups of the mud easily. The current backup directory \
is set to: `cat ./backup.dir`. You may change this below. If you need to restore \
from a backup, however, you'll need to do that part yourself. To uncompress a backed \
up file, use tar -zxvf 'filename', and then move the file you need restored back.\n\n\
  Please choose a backup option: " 0 0 7 \
  "All"      "Includes all options below" \
  "Players"  "Backup the player directory" \
  "Areas"    "Backup the areas directory" \
  "Source"   "Backup the source code" \
  "Other"    "Backup everything in the class, msgbase and data directories." \
  "Change"   "Move the backup directory somewhere else." \
  "Return"   "Return to the Main Menu." 2> $tempfile
  
menuitem=`cat $tempfile`
opt=$?
  
case $menuitem in
  "All")
     echo Backing up everything.
	 /bin/sh ./backup_mud.sh all;;
  "Players")
     echo Backing up the Pfiles.
	 /bin/sh ./backup_mud.sh player;;
  "Areas")
     echo Backing up the areas.
	 /bin/sh ./backup_mud.sh area;;
  "Source")
     echo Backing up the source code.
	 /bin/sh ./backup_mud.sh source;;
  "Other")
     echo Backing up the Msgbase, Class and Data directories.
	 /bin/sh ./backup_mud.sh other;;
  "Change")
     $DIALOG --clear --no-shadow --title "Backup Directory" \
     --inputbox "Please specify a backup directory.\nEnter it in this format:\n\
	 /home/mud/mybackups\n\
	 That is, a full directory to which you have access, with no trailing slash. \
	 Enter a Directory for Backups:\n" 0 0 2>$tempfile
	 case $? in
	 0)
	   mv `cat ./backup.dir`/* `cat $tempfile`/
	   rm ./backup.dir
	   echo "`cat $tempfile`" >> ./backup.dir;;
	 esac;;									   
  "Return")
     echo Returning to main menu!
     exit 0;;
esac

	 
	 

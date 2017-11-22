#!/bin/sh
#
# This worked fine in Redhat 7.3, but refuses to work in FreeBSD. Not
# sure of the cause.
#
DIALOG=${DIALOG=dialog}
tempfile=tempfile 2>/dev/null || tempfile=/tmp/test$$
trap "rm -f $tempfile" 0 1 2 5 15

if test -e ./bin/first.run;
then
{
   $DIALOG --clear --no-shadow --title "Welcome to SunderMud 2.0" \
   --msgbox "Welcome to SunderMud 2.0!\n\n\
   I see that this is the first time you've run the Control Panel, so we have a few \
   things to take care of first, and then you'll be up and running!\n\n\
   First of all, you must read and agree to the license. You inidcate agreement\
   by hitting enter on the next screen. If you don't agree, exit with CTRL-C." 0 0 2>$tempfile
   case $? in
      0)
	  echo Viewing License....;;
	  255)
	  echo Aborting;
	  exit 1;
   esac
   cd ./bin/
   /bin/sh ./viewlicense.sh
   sleep 1
   $DIALOG --clear --no-shadow --title "Next.... Backup Directory"\
   --inputbox "Thank you for taking the time to read the license. Now we are\
   going to get down to the dirt. Several items need to be configured before we\
   the mud can be started.\n\n\
   First of all, you need to specify a directory to put your backup files in.\n\
   Enter it in this format:\n\
   /home/mud/mybackups\n\
   That is, a full directory to which you have access, with no trailing slash. If\
   you need to change this later, edit the file in bin/backup.dir.\n\n\
   Enter a Directory for Backups:\n" 0 0 2>$tempfile   
   case $? in
   0)
     rm ./backup.dir
     echo "`cat $tempfile`" >> ./backup.dir;;
   esac
   sleep 1
   /bin/sh ./autosetup.sh
   sleep 1
   $DIALOG --clear --no-shadow --title "Finished First-time Config"\
   --msgbox "Congratulations! You're ready to compile and run your mud, which\
   you can do at the next menu. In the future, when you run this program, you\
   will be taken directly to the automagically." 0 0
   case $? in
   0)
     echo Executing Control Panel Main Menu...
	 rm -rf ./first.run
	 cd ../;;
   255)
     echo Leaving already?
	 exit 1;;
   esac
}
fi

while true
do

$DIALOG --clear --no-shadow --item-help --title "SunderMud 2.0 Control Panel" \
--menu "Welcome to SunderMud 2.0!\n\n\
This control panel tries to put in one place the various things you need \
to manage the mud externally.\n\n\
  Please make a selection:" 0 0 8 \
  "Launch"    "Runs SunderMud"         "Launches the startup script in the background." \
  "Config"    "Configure SunderMud"    "Setups up the bin/sunder.rc and src/options.h files." \
  "Make"      "Compiles the Mud"       "Builds the mud from current sources." \
  "Backup"    "Run Backups"            "Backs up the mud" \
  "License"   "View License"           "Views Sunder, Rom, Merc and Diku licenses." \
  "Quit"      "Exit the Panel"         "What? Finished already?" 2> $tempfile

menuitem=`cat $tempfile`

opt=$?

case $menuitem in
   "Launch")
     echo Running the mud.
	 cd ./bin
     ./startup &
	 cd ../;;
   "Config")
     echo Launching autosetup
	 cd ./bin
	 /bin/sh ./autosetup.sh
	 cd ../;;
   "Make")
     echo Compiling the mud.... This might be a bit.
	 cd ./src
	 make
	 cd ../;;
   "Backup")
     echo Backing up the Mud.
	 cd ./bin
	 /bin/sh ./backupmenu.sh
	 cd ../;;
   "License")
     echo Viewing the License.
	 cd ./bin
	 /bin/sh ./viewlicense.sh
	 cd ../;;
   "Quit")
     echo Goodbye!
	 clear
	 exit 0;;
esac

sleep 1
done

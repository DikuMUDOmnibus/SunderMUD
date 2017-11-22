#!/bin/sh

# backup_mud.sh
# 4/7/97
# Zeran
# Written to backup source, areas, and players and store in 
# /home/admin/mudbackups/ appropriately.
# Usage:  backup_mud.sh [ all | source | area | player | other ]
# No argument defaults to choice "all"
# "all" makes backup of all mud source, player, and area files
# "source" makes backup of source files only
# "area" makes backup of area files only
# "player" makes backup of player files only
# 11/9/97
# Zeran
# Changed /home/admin/mudbackups to a mounted zip disk for security
# 4/26/98
# Zeran
# Modified for use with SunderMud ][
# Changed to use separate filesystem on harddrive...
# "other" option in usage...archives msgbase, data, and class directories
# 5/5/98
# Zeran
# Modified to create archive in area, player, and src directories so that
# the "../" portion of the path isn't put in the archive.
# 6/25/02
# Lotherius
# Removed some of Z's profanity :) Modified to work better with the new scripts.
# Specifically, modified it to read a config file of where to make the backups.
# Also set it up to create directories that don't exit... Duh.

dir_prefix="`cat backup.dir`"
all_files="FALSE"
valid_arg="FALSE"

# get date
date_string=`date +%m%d%y`

# verify in bin directory by looking for backup.dir file in current directory
if [ ! \( -f "backup.dir" \) ] 
	then
	echo "Can't find backup.dir to specify backup location. Please run"
	echo "the included autosetup script, and then execute this script"
	echo "from the binary directory only. Thanks."
	exit 1
fi

# Check mount point 
if [  ! \( -d "$dir_prefix" \) ]
then
	/usr/bin/clear
	/bin/df
	echo
	echo "I can't find $dir_prefix ... It either doesn't exist, or needs to mounted if a removable device"
	echo "Attempting to create. This will fail, of course, if you have a removable device that isn't mounted."
	mkdir ${dir_prefix}
	echo
fi

choice=$1
if [ "$choice" = "" ]
	then
	choice="all"
fi

# Clear screen
/usr/bin/clear 

if [ "$choice" = "all" ]
	then
	echo "*********************************"
	echo "Running FULL backup of all files."
	echo "*********************************"
	all_files="TRUE"
	valid_arg="TRUE"
	sleep 2
fi

# Area file backup section
if [ "$choice" = "area" -o "$all_files" = "TRUE" ]
	then
	valid_arg="TRUE"
	echo "*********************************"
	echo "Running AREA file backup."
	echo "*********************************"
	echo
	
	# Remove leftover core files
	if [ -f "../area/core*" ]
		then
		echo "A core file has been found in the area directory, moving it"
		echo "to ../core ..."
		mv ../area/core* ../core*
		fi
	if [ -f "${dir_prefix}/allarea.tgz.${date_string}" ]
		then
		echo "An area file backup has already been created today."
		echo "Please rename or remove the old backup and then run this script again."
		echo
		sleep 2
	else
		echo "Creating archive file allarea.tgz.${date_string}"
		cd ../area
		tar cvfz ./allarea.tgz * 
		echo "Moving file to archive directory."
		mv allarea.tgz ${dir_prefix}/allarea.tgz.${date_string}
		chmod 440 ${dir_prefix}/allarea.tgz.${date_string}
		echo "Area file backup complete."
		echo 
		sleep 2
	fi

	if [ "$all_files" = "FALSE" ]
		then 
		exit 0
	fi
fi

# Source file backup section
if [ "$choice" = "source"  -o "$all_files" = "TRUE" ]
	then
	valid_arg="TRUE"
	echo "*********************************"
	echo "Running SOURCE file backup."
	echo "*********************************"
	echo
	if [ -f "${dir_prefix}/src/allsrc.tgz.${date_string}" ]
		then
		echo "A source file backup has already been created today."
		echo "Please rename or remove the old backup and then run this script again."
		echo
		sleep 2
	else
		echo "Creating archive file allsrc.tgz.${date_string}"
		cd ../src
		tar cvfz ./allsrc.tgz *.[ch] 
		echo "Moving file to archive directory."
		mv allsrc.tgz ${dir_prefix}/allsrc.tgz.${date_string}
		chmod 440 ${dir_prefix}/allsrc.tgz.${date_string}
		echo "Copying makefiles and building script..."
		cp build_sunder ${dir_prefix}/build_sunder
		cp Make.sunder ${dir_prefix}/Make.sunder
		echo "Source file backup complete."
		echo 
		sleep 2
	fi

	if [ "$all_files" = "FALSE" ]
		then 
		exit 0
	fi
fi

# Player file backup section
if [ "$choice" = "player"  -o "$all_files" = "TRUE" ]
	then
	valid_arg="TRUE"
	echo "*********************************"
	echo "Running PLAYER file backup." 
	echo "*********************************"
	echo
	if [ -f "${dir_prefix}/player/allplayer.tgz.${date_string}" ]
		then
		echo "A player file backup has already been created today."
		echo "Please rename or remove the old backup and then run this script again."
		echo 
		sleep 2
	else
		echo "Creating archive file allplayer.tgz.${date_string}"
		cd ../player
		tar cvfz ./allplayer.tgz * 
		echo "Moving file to archive directory."
		mv allplayer.tgz ${dir_prefix}/allplayer.tgz.${date_string}
		chmod 440 ${dir_prefix}/allplayer.tgz.${date_string}
		echo "Player file backup complete."
		echo 
		sleep 2
	fi	
fi

if [ "$choice" = "other"  -o "$all_files" = "TRUE" ]
	then
	valid_arg="TRUE"
	echo "*********************************"
	echo "Running OTHER files backup." 
	echo "*********************************"
	echo
	if [ -f "${dir_prefix}/other/msgbase_class_data.tgz.${date_string}" ]
		then
		echo "An other files backup has already been created today."
		echo "Please rename or remove the old backup and then run this script again."
		echo 
		sleep 2
	else
		echo "Creating archive file msgbase_class_data.tgz.${date_string}"
		tar cvfz ./msgbase_class_data.tgz ../msgbase ../class ../data
		echo "Moving file to archive directory."
		mv msgbase_class_data.tgz ${dir_prefix}/msgbase_class_data.tgz.${date_string}
		chmod 440 ${dir_prefix}/msgbase_class_data.tgz.${date_string}
		echo "Other files backup complete."
		echo 
		sleep 2
	fi	
fi

if [ "$valid_arg" = "FALSE" ]
	then
	echo "******"
	echo "Usage:  backup_mud.sh [ all | area | source | player | other ]"
	echo "******"
fi

exit 0

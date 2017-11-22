#!/bin/sh
# strip_pfile_field.sh
# 05/09/98 
# Zeran
# This script is a utility for removing unwanted lines from all player
# files in the player directory.  It is useful for when fields are removed
# from the mud that are normally stored in the pfile, or when the names
# of the fields in the pfile need to be changed.
# Usage:  strip_pfile_field [pfile_field_name]
#   ex:  strip_pfile_field Amod
#   would remove all lines in all pfiles with "Amod" in the line
# Use extreme care when using this script.  A backup of the player
# files before running this script is *highly* recommended.

# Verify running this script from the bin directory
if [ ! \( -f "bin.dir" \) ]
then
	echo "Run this script from the bin directory only."
	echo
	exit 1
fi

# Verify only one argument
if [ $# -gt 1 ]
then
	echo "Only specify *one* field to remove please." 
	echo
	exit 1
fi
if [ $# -eq 0 ]
then
	echo "Usage: strip_pfile_field [pfile_field_name]"
	echo
	exit 1
fi


# Change to the player directory
cd ../player

# gunzip all the pfiles
gunzip *.gz

# Move the current_player_list outside this directory temporarily
mv current_player_list ..

# Now strip the files
strip_field="$1"
for pfile in *
do
	cat $pfile | grep -v "$strip_field" > $pfile
done

# Move the current_player_list back in to this directory
mv ../current_player_list .

# Finished...
echo "Finished, please take a minute to look over the pfiles."
exit 0

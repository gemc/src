#!/bin/zsh

# Purpose: creates a doxyfile for a specific class, based on a doxyfile created with doxygen -g
#
# Arguments: exactly one argument, the name of the library

if [ "$#" -ne 1 ]; then
	echo " "       >&2
	echo " Usage:" >&2
	echo " "       >&2
	echo  " $0 libraryName"  >&2
	echo " "       >&2
	exit 1
fi


library=$1
echo " Setting Doxyfile for "$library


# no PROJECT_BRIEF
sed -i s/template_class/$library/g Doxyfile

toCheck=(PROJECT_NAME OPTIMIZE_OUTPUT_FOR_C BUILTIN_STL_SUPPORT SHOW_FILES HTML_COLORSTYLE_GAMMA QUIET)
echo " "Doxyfile Main changes:
echo
for l in $toCheck
do
	setting=$(grep "$l" Doxyfile | grep -v "#")
	echo "  - "$setting
done

echo
echo " Copying doxygen.css to "$library
cp ../docs/doxygen.css .

	if [[ -v TRAVISENVIRONMENT ]]; then
		echo " Travis Build" >&2
		# Note: EXCLUDE_PATTERNS includes glibrary because we're using this script for "gemc" on travis as well, and in there we clone glibrary
		sed -i 's/EXCLUDE_PATTERNS       =/EXCLUDE_PATTERNS       = \*moc\* \*glibrary\* \*\/html\/\*/g'    Doxyfile
		cp glibrary/docs/mylayout.css .
	else
		echo " Non travis Build" >&2
		# Note: normal EXCLUDE_PATTERNS, glibrary is not here
		sed -i 's/EXCLUDE_PATTERNS       =/EXCLUDE_PATTERNS       = \*moc\* \*\/html\/\*/g'    Doxyfile
		cp ../glibrary/docs/mylayout.css .
	fi

rm -f Doxyfilebackup

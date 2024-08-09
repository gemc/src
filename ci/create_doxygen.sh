#!/bin/zsh


# Purpose: creates a doxyfile using the doxygen -g command to get the latest doxygen goodies)
# The replace each line with the ones shown below

echo " Creating System Doxyfile "
doxygen -g &> /dev/null

cp Doxyfile DoxyfilePure
class=$1


# painfully, manually doing these, but we HAVE TO
# Note: keep STRIP_CODE_COMMENTS (refers just to doxygen comments) and SOURCE_TOOLTIPS to yes to make browing more readable
sed -i "s/PROJECT_NAME           = \"My Project\"/PROJECT_NAME  = \"$class\"/g"                     Doxyfile
sed -i 's/REPEAT_BRIEF           = YES/REPEAT_BRIEF           = NO/g'                               Doxyfile
sed -i 's/OPTIMIZE_OUTPUT_FOR_C  = NO/OPTIMIZE_OUTPUT_FOR_C   = YES/g'                              Doxyfile
sed -i 's/BUILTIN_STL_SUPPORT    = NO/BUILTIN_STL_SUPPORT     = YES/g'                              Doxyfile
#sed -i 's/SHOW_INCLUDE_FILES     = YES/SHOW_INCLUDE_FILES     = NO/g'                               Doxyfile
#sed -i 's/SHOW_FILES             = YES/SHOW_FILES             = NO/g'                               Doxyfile
sed -i 's/EXTRACT_ALL            = NO/EXTRACT_ALL            = YES/g'                               Doxyfile
sed -i 's/SOURCE_BROWSER         = NO/SOURCE_BROWSER          = YES/g'                              Doxyfile
sed -i 's/RECURSIVE              = NO/RECURSIVE               = YES/g'                              Doxyfile
sed -i 's/HTML_COLORSTYLE_HUE    = 220/HTML_COLORSTYLE_HUE    = 200/g'                              Doxyfile
sed -i 's/HTML_COLORSTYLE_SAT    = 100/HTML_COLORSTYLE_SAT    = 240/g'                              Doxyfile
sed -i 's/HTML_COLORSTYLE_GAMMA  = 80/HTML_COLORSTYLE_GAMMA   = 190/g'                              Doxyfile
sed -i 's/HTML_DYNAMIC_MENUS     = YES/HTML_DYNAMIC_MENUS     = YES/g'                              Doxyfile
sed -i 's/GENERATE_TREEVIEW      = NO/GENERATE_TREEVIEW       = YES/g'                              Doxyfile
sed -i 's/FORMULA_FONTSIZE       = 10/FORMULA_FONTSIZE        = 14/g'                               Doxyfile
sed -i 's/DOT_FONTNAME           = Helvetica/DOT_FONTNAME     = Avenir/g'                           Doxyfile
sed -i 's/DOT_FONTSIZE           = 10/DOT_FONTSIZE            = 16/g'                               Doxyfile
sed -i 's/TEMPLATE_RELATIONS     = NO/TEMPLATE_RELATIONS      = YES/g'                              Doxyfile
sed -i 's/GENERATE_LATEX         = YES/GENERATE_LATEX         = NO/g'                               Doxyfile
sed -i 's/HAVE_DOT               = NO/HAVE_DOT                = YES/g'                              Doxyfile
sed -i 's/UML_LOOK               = NO/UML_LOOK                = YES/g'                              Doxyfile
sed -i 's/EXCLUDE                =/EXCLUDE                    = README.md .git/g'                   Doxyfile
sed -i 's/QUIET                  = NO/QUIET                   = YES/g'                              Doxyfile
sed -i 's/HIDE_FRIEND_COMPOUNDS  = NO/HIDE_FRIEND_COMPOUNDS   = YES/g'                              Doxyfile
sed -i "s/HTML_EXTRA_STYLESHEET  =/HTML_EXTRA_STYLESHEET      = ..\/mydoxygen.css/g"                 Doxyfile
sed -i 's/PREDEFINED             =/PREDEFINED                 = DOXYGEN_SHOULD_SKIP_THIS/g'         Doxyfile


# painfully removing the individual patterns
sed -i  '/*.hpp \\/d'      Doxyfile
sed -i  '/*.java \\/d'     Doxyfile
sed -i  '/*.ii \\/d'       Doxyfile
sed -i  '/*.ixx \\/d'      Doxyfile
sed -i  '/*.ipp \\/d'      Doxyfile
sed -i  '/*.i++ \\/d'      Doxyfile
sed -i  '/*.inl \\/d'      Doxyfile
sed -i  '/*.cs \\/d'       Doxyfile
sed -i  '/*.d \\/d'        Doxyfile
sed -i  '/*.m \\/d'        Doxyfile
sed -i  '/*.doc \\/d'      Doxyfile
sed -i  '/*.txt \\/d'      Doxyfile
sed -i  '/*.f \\/d'        Doxyfile
sed -i  '/*.for \\/d'      Doxyfile
sed -i  '/*.vhd \\/d'      Doxyfile
sed -i  '/*.ucf \\/d'      Doxyfile
sed -i  '/*.md \\/d'       Doxyfile
sed -i  '/*.mm \\/d'       Doxyfile
sed -i  '/*.dox \\/d'      Doxyfile
sed -i  '/*.f90 \\/d'      Doxyfile
sed -i  '/*.f95 \\/d'      Doxyfile
sed -i  '/*.f03 \\/d'      Doxyfile
sed -i  '/*.f08 \\/d'      Doxyfile
sed -i  '/*.markdown \\/d' Doxyfile
sed -i  '/*.tcl \\/d'      Doxyfile
sed -i  '/*.vhdl \\/d'     Doxyfile
sed -i  '/*.qsf \\/d'      Doxyfile
sed -i  '/*.php \\/d'      Doxyfile
sed -i  '/*.php4 \\/d'     Doxyfile
sed -i  '/*.php5 \\/d'     Doxyfile
sed -i  '/*.phtml \\/d'    Doxyfile
sed -i  '/*.idl \\/d'      Doxyfile
sed -i  '/*.ddl \\/d'      Doxyfile
sed -i  '/*.odl \\/d'      Doxyfile

rm -f Doxyfilebackup

toCheck=(PROJECT_NAME OPTIMIZE_OUTPUT_FOR_C BUILTIN_STL_SUPPORT SHOW_FILES HTML_COLORSTYLE_GAMMA QUIET)
echo " "Doxyfile Main changes:
echo
for l in $toCheck
do
	setting=$(grep "$l" Doxyfile | grep -v "#")
	echo "  - "$setting
done

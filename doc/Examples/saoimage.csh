#!/bin/csh -f
#
# Strips the parenthesis out of the color file created by saoimage.
# Replaces joining pairs by a comma-space.
#
#  History:
#    19jun95 jm  Original script.
#
set task = $0
if ($#argv == 0) then
    echo "Usage: ${task:t} file"
    echo "Converts one saoimage color table file"
    echo "into a table that can be read by WIP."
    echo "      Example:    ${task:t} sao.table >outputfile"
    echo " "
    echo "NOTE:  This creates the reverse LUT.  To generate"
    echo "the standard LUT, send the output of this program"
    echo "to the function sort; as in the following example:"
    echo "      Example:    ${task:t} sao.table | sort -nr +0 >outputfile"
    echo " "
    exit 0
endif
#
set tempfile=/tmp/tempawkfile.$$
cat >$tempfile <<END_OF_AWK_FILE
BEGIN {MAXCOLORS = 1001; color = "0"; nred = ngreen = nblue = 0}

(\$1 ~ /^#/) {next;}  # Ignore comment lines.

((color == "0") && (\$1 ~ /^RED:/)) {color = "R"; next;}

((color == "R") && (\$1 ~ /^GREEN:/)) {color = "G"; next;}

((color == "R") && (NF > 1)) {redx[nred] = \$1; redy[nred] = \$2; nred++;}

((color == "G") && (\$1 ~ /^BLUE:/)) {color = "B"; next;}

((color == "G") && (NF > 1)) {grenx[ngreen] = \$1; greny[ngreen] = \$2; ngreen++;}

((color == "B") && (NF > 1)) {bluex[nblue] = \$1; bluey[nblue] = \$2; nblue++;}

END {y = rlim = glim = blim = 0; dop = 1; first = 1;
  while (((rlim + 1) <   nred) && ( redx[rlim+1] ==  redx[rlim])) {rlim++;}
  while (((glim + 1) < ngreen) && (grenx[glim+1] == grenx[glim])) {glim++;}
  while (((blim + 1) <  nblue) && (bluex[blim+1] == bluex[blim])) {blim++;}
  for (j = 0; j < MAXCOLORS; j++) {
    x = j / (MAXCOLORS - 1);
    if (((rlim + 1) <   nred) && (x >=   redx[rlim+1])) {
      rlim++; dop = 1;
      while ((rlim < nred) && (redx[rlim] == redx[rlim-1])) {rlim++;}
      y = redx[rlim];
    }
    if (((glim + 1) < ngreen) && (x >= grenx[glim+1])) {
      glim++; dop = 1;
      while ((glim < ngreen) && (grenx[glim] == grenx[glim-1])) {glim++;}
      y = grenx[glim];
    }
    if (((blim + 1) <  nblue) && (x >=  bluex[blim+1])) {
      blim++; dop = 1;
      while ((blim < nblue) && (bluex[blim] == bluex[blim-1])) {blim++;}
      y = bluex[blim];
    }
    if (dop > 0) {
      if (first > 0) {
       printf("# RED GREEN  BLUE  LEVEL\n");
       first = 0;
     }
     printf("%.3f %.3f %.3f  %.3f\n", redy[rlim], greny[glim], bluey[blim], y);
    }
    dop = 0;
  }
}
END_OF_AWK_FILE
#
onintr clearfile
set file=$1
shift argv
sed -e 's/)(/@/g' $file | awk -F@ '{for(i=1;i<=NF;i++)print $i}' |\
sed -e 's/(//g' | sed -e 's/)//g' | sed -e 's/,/ /g' | awk -f $tempfile $* -
#
clearfile:
rm -fr $tempfile
exit 0

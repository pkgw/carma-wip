# Remove everything between BEGIN and END index comments.
/<\!--.*WIP BEGIN Index/,/<\!--.*WIP END Index/d
/<\!--.*WIP BEGIN Table/,/<\!--.*WIP END Table/d
# Stuff to ignore
s?<HTML[^>]*>??g
s?</HTML[^>]*>??g
s?<HEADER[^>]*>??g
s?</HEADER[^>]*>??g
s?<BODY[^>]*>??g
s?</BODY[^>]*>??g
s?<ISINDEX>??
s?</ADDRESS>??g
s?<NEXTID[^>]*>??g
s?<HR[^>]*>??g
s?<BR[^>]*>??g
s?<TITLE>[^<]*</TITLE>??g
s?<H1>[^<]*</H1>??g
# Too bad there's no way to make sed ignore case!
# character set translations for LaTex special chars
s?{?\\{?g
s?}?\\}?g
s?&gt.?\\verb+>+?g
s?&lt.?\\verb+<+?g
s/\\\([01-9][01-9]\)/\\esc{\1}/g
s/\\\([01-9]\)/\\esc{\1}/g
s/\\\(f[nris]\)/\\esc{\1}/g
#
# s?\\?\\backslash ?g
s?%?\\%?g
s?\$?\\$?g
s?&?\\&?g
s?#?\\#?g
s?_?\\_?g
s?~?\\~?g
s?\^?\\verb+^+?g
# For table, etc. references, Usage (refitem can only be one word):
# <!-- "WIP: refitem rest of string" -->
# produces
# rest of string~\ref{refitem}.
s?<\!--.*WIP: *\([^ ]*\) \([^"]*\)"[^>]*>?	\2~\\ref{\1}.?g
# Change font type.
s/<STRONG[^>]*>\([^<]*\)<\/STRONG>/{\\bf \1}/g
# Deal with each help entry:
/<DT>/,/<DD>/s/\([^-]*\)-\(.*\)/\\item [{\\tt \1} --]\2\\\\/
s?<DT>??g
s?<DD>??g
#  Before changing preformatted code to verbatim mode,
#  change some of the LaTeX items to verbatim entries.
/<PRE>/,/<\/PRE>/s?\\verb+^+?\^?g
/<PRE>/,/<\/PRE>/s?\\&?\&?g
/<PRE>/,/<\/PRE>/s?\\%?%?g
/<PRE>/,/<\/PRE>/s?\\esc{\([^}]*\)}?\\\1?g
#  Convert preformatted code to verbatim mode.
s?<PRE>?\\begin{verbatim}?g
s?</PRE>?\\end{verbatim}?
#  Convert code extracts and anchors to \tt text.  The first
#  item converts \tt double quotes to a special string that
#  will be fixed later on.
/<CODE[^>]*>/s/"/@PAR/g
s/<A[^>]*><CODE>\([^<]*\)<\/CODE><\/A>/{\\tt \1}/g
s/<A[^>]*>\([^<]*\)<\/A>/{\\tt \1}/g
s/<CODE[^>]*>\([^<]*\)<\/CODE>/{\\tt \1}/g
s/<A[^>]*>//g
# Paragraph borders
s?<[Pp]>?\\par ?g
s?</[Pp]>??g
# Headings
s?<DL>??g
s?</DL>??g
#  Handle special groupings.
s?\\{\\}?\\{\\,\\}?g
s?\[\]?[\$\\:\$]?g
s?()?(\\,)?g
#  Convert matching quotes to LaTeX format.
s/"\([^"]*\)"/``\1''/g
s/@PAR/"/g
#  Convert ... to \dots
s/\.\.\./\\dots/g
#  Get rid of blank lines.
/^$/d

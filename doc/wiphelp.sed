# Remove everything up to the END comments.
1,/<\!--.*WIP END Index/c\
\\begin{rawhtml}
# More Stuff to ignore
s?</BODY[^>]*>??g
s?</HTML[^>]*>?\\end{rawhtml}?g

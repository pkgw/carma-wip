# 	Makefile for the (LaTeX) WIP manual
#	10jun91 jm  Created for WIP.
#	14jun95 jm  Heavily modified; also added HTML usage.
#
F=wip
TEX = wip.tex
STY = wip.sty
INC = basic.tex  cmdline.tex  cmdname.tex concepts.tex disclaimer.tex \
	faq.tex      fit.tex     flow.tex glossary.tex      howto.tex \
	iface.tex images.tex   macros.tex  samples.tex    uservar.tex \
	wipinit.tex
SED = html2latex.sed html2help.sed wiphelp.sed wiphelp.html
SUBMIT = $(INC) $(STY) $(TEX) $(SED) Makefile Examples
PLOTS = basic cimage cos fit greek hershey image image2 \
	overlay panel simple symbol text
CPLOTS = cimage palettes

help:
	@echo "Makefile for the WIP manual using LaTeX"
	@echo "======================================="
	@echo "Macro names:"
	@echo "  F, TEX, STY, INC, SUBMIT"
	@echo "Targets:"
	@echo "  help		This help"
	@echo "  $(F)           Make the dvi file for manual F=$(F)"
	@echo "  examples       Make hardcopies of example plots"
	@echo "  tar            Make a tar file of the documentation"
	@echo "  dat            Make the file ../wiphelp.dat."
	@echo "  clean [F=$(F)] Like cleanall, but only for $(F).* files"
	@echo "  cleanall       Clean all .dvi, .aux, etc. TeX related files"
	@echo " "
	@echo "The used TEX= files in this directory are:"
	@echo "  $(TEX)"
	@echo "The used STY= files in this directory are:"
	@echo "  $(STY)"
	@echo "The used INC= files in this directory are:"
	@echo "  $(INC)"
	@echo "The used PLOTS= files in ./Examples are:"
	@echo "  $(PLOTS)"
	@echo "The used (color) CPLOTS= files in ./Examples are:"
	@echo "  $(CPLOTS)"

cleanall:
	@echo "Cleaning all (La)TeX related documents..."; sleep 2
	rm -f *.aux *.dvi *.idx *.lof *.log *.lot *.ps *.toc *.tmp
	rm -rf tmp

clean:
	@echo "Cleaning all (La)TeX related documents for F=$(F)..."; sleep 2
	rm -f $(F).aux $(F).dvi $(F).idx $(F).lof $(F).log $(F).lot \
	    $(F).ps $(F).toc
	@echo "The tmp files have not been deleted; only in 'make cleanall'."

cmdname.tex: lhelp.tex hhelp.tex

lhelp.tex: html2latex.sed wiphelp.html
	sed -f html2latex.sed wiphelp.html > lhelp.tex

hhelp.tex: wiphelp.sed wiphelp.html
	sed -f wiphelp.sed wiphelp.html > hhelp.tex

$(F): $(F).ps

$(F).ps: $(F).dvi
	dvips $(F) -o

$(F).dvi: $(TEX) $(INC) $(STY) $(F).ind lhelp.tex hhelp.tex
	@echo "Running latex 3 times - will take a while..."
	latex $(F) 
	latex $(F) 
	latex $(F)

examples:
	@echo "Make the WIP examples:"
	(cd Examples; wip.csh $(PLOTS); wip.csh -d /vcps $(CPLOTS);)

#
#  Converts wiphelp.html into ../wiphelp.dat.  It first filters
#  out most of the HTML codes (via html2help.sed); changes lines
#  between <PRE> and </PRE> to begin with a dot (.); runs fmt
#  on the text (lines that start with a dot are not formatted);
#  converts the dot back to a tab; and finally removes any blank
#  lines or lines containing the <PRE> and </PRE> directives.
#
#  Use:
# grep '^[A-Z]' wiphelp.html | awk '{printf("<A HREF=#%s>%s</A> -\n", $1, $1)}'
#  to generate the index listing at the top of the file.
#
dat: ../wiphelp.dat

../wiphelp.dat: html2help.sed wiphelp.html
	sed -f html2help.sed wiphelp.html |\
	sed -e '/<PRE>/,/<\/PRE>/s/\(.*\)/\.\1/g' | fmt -79 |\
	sed -e '/<PRE>/,/<\/PRE>/s/\.\(.*\)/	\1/g' | \
	sed -e '/^$$/d' -e '/<PRE>/d' -e '/<\/PRE>/d' > ../wiphelp.dat

tar:
	tar cvf $(F).tar $(SUBMIT)

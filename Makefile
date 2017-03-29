all: pdf html explanatory.pdf explanatory.html

pdf: README.md
	pandoc README.md -o README.pdf -H header.tex --number-sections -o D0443R2_A_Unified_Executors_Proposal.pdf

html: README.md
	pandoc README.md -o README.html --number-sections -o D0443R2_A_Unified_Executors_Proposal.html

PANDOC_FLAGS = -f markdown \
	       --smart

CITEPROC= --filter pandoc-citeproc \
	  --csl=acm-sig-proceedings-long-author-list.csl

explanatory.pdf: explanatory.md explanatory_header.tex explanatory_metadata.yaml
	pandoc $(PANDOC_FLAGS) $(CITEPROC) -H explanatory_header.tex explanatory_metadata.yaml explanatory.md -o explanatory.pdf

explanatory.html: explanatory.md explanatory_header.tex explanatory_metadata.yaml
	pandoc $(PANDOC_FLAGS) $(CITEPROC) -H explanatory_header.tex explanatory_metadata.yaml explanatory.md -o explanatory.html

clean:
	rm -f *.pdf README.html explanatory.html


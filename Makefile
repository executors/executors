all: wording explanatory

wording: wording_pdf wording_html

explanatory: explanatory_pdf explanatory_html

wording_pdf: wording.md
	pandoc wording.md -o wording.pdf -H header.tex --number-sections -o D0443R2_A_Unified_Executors_Proposal.pdf

wording_html: wording.md
	pandoc wording.md -o wording.html --number-sections -o D0443R2_A_Unified_Executors_Proposal.html

PANDOC_FLAGS = -f markdown \
	       --smart

CITEPROC= --filter pandoc-citeproc \
	  --csl=acm-sig-proceedings-long-author-list.csl

explanatory_pdf: explanatory.md explanatory_header.tex explanatory_metadata.yaml
	pandoc $(PANDOC_FLAGS) $(CITEPROC) -H explanatory_header.tex explanatory_metadata.yaml explanatory.md -o DXXXXR0_Executors_Explained.pdf

explanatory_html: explanatory.md explanatory_header.tex explanatory_metadata.yaml
	pandoc $(PANDOC_FLAGS) $(CITEPROC) -H explanatory_header.tex explanatory_metadata.yaml explanatory.md -o DXXXXR0_Executors_Explained.html

clean:
	rm -f D0443*.pdf DXXXX*.pdf D0443*.html DXXXX*.html


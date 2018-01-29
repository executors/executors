all: wording explanatory

wording: wording_pdf wording_html

explanatory: explanatory_pdf explanatory_html

wording_pdf: wording_front_matter.md wording.md
	pandoc wording_front_matter.md wording.md -o wording.pdf -H header.tex --number-sections -o P0443R5_A_Unified_Executors_Proposal.pdf

wording_html: wording.md
	pandoc wording_front_matter.md wording.md -o wording.html --number-sections -o P0443R5_A_Unified_Executors_Proposal.html

PANDOC_FLAGS = -f markdown \
	       --variable urlcolor=cyan

CITEPROC= --filter pandoc-citeproc \
	  --csl=acm-sig-proceedings-long-author-list.csl

explanatory_pdf: explanatory.md explanatory_header.tex explanatory_metadata.yaml
	pandoc $(PANDOC_FLAGS) $(CITEPROC) -H explanatory_header.tex explanatory_metadata.yaml explanatory.md -o P0761R1_Executors_Design_Document.pdf

explanatory_html: explanatory.md explanatory_header.tex explanatory_metadata.yaml
	pandoc $(PANDOC_FLAGS) $(CITEPROC) -H explanatory_header.tex explanatory_metadata.yaml explanatory.md -o P0761R1_Executors_Design_Document.html

clean:
	rm -f *0443*.pdf *0443*.html *0761*.pdf *0761*.html


all: wording explanatory simplification

wording: wording_pdf wording_html

explanatory: explanatory_pdf explanatory_html

simplification: simplification_pdf simplification_html

wording_pdf: wording_front_matter.md wording.md
	pandoc wording_front_matter.md wording.md -o wording.pdf -H header.tex --number-sections -o D0443R2_A_Unified_Executors_Proposal.pdf

wording_html: wording.md
	pandoc wording_front_matter.md wording.md -o wording.html --number-sections -o D0443R2_A_Unified_Executors_Proposal.html

PANDOC_FLAGS = -f markdown \
	       --variable urlcolor=cyan

CITEPROC= --filter pandoc-citeproc \
	  --csl=acm-sig-proceedings-long-author-list.csl

explanatory_pdf: explanatory.md explanatory_header.tex explanatory_metadata.yaml
	pandoc $(PANDOC_FLAGS) $(CITEPROC) -H explanatory_header.tex explanatory_metadata.yaml explanatory.md -o D0676R0_Executors_Explained.pdf

explanatory_html: explanatory.md explanatory_header.tex explanatory_metadata.yaml
	pandoc $(PANDOC_FLAGS) $(CITEPROC) -H explanatory_header.tex explanatory_metadata.yaml explanatory.md -o D0676R0_Executors_Explained.html

simplification_pdf: simplification_proposal.md
	pandoc $(PANDOC_FLAGS) $(CITEPROC) -H simplification_proposal_header.tex simplification_proposal.md -o N0688R0_simplification_proposal.pdf

simplification_html: simplification_proposal.md
	pandoc $(PANDOC_FLAGS) $(CITEPROC) --number-sections simplification_proposal.md -o N0688R0_simplification_proposal.html

clean:
	rm -f D0443*.pdf *0676*.pdf D0443*.html *0676*.html D0688*.pdf D0688*.html


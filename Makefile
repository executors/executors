all: wording explanatory dependent_wording

wording: wording_pdf wording_html

dependent_wording: dependent_wording_pdf dependent_wording_html

explanatory: explanatory_pdf explanatory_html

wording_pdf: wording_front_matter.md wording.md
	pandoc wording_front_matter.md wording.md -o wording.pdf -H header.tex --number-sections -o P0443R10_A_Unified_Executors_Proposal.pdf

wording_html: wording.md
	pandoc wording_front_matter.md wording.md -o wording.html --number-sections -o P0443R10_A_Unified_Executors_Proposal.html

dependent_wording_pdf: dependent_wording_front_matter.md dependent_wording.md
	pandoc dependent_wording_front_matter.md dependent_wording.md -o dependent_wording.pdf -H header.tex --number-sections -o P1244R0_Dependent_Execution_for_a_Unified_Executors_Proposal.pdf

dependent_wording_html: dependent_wording.md
	pandoc dependent_wording_front_matter.md dependent_wording.md -o dependent_wording.html --number-sections -o P1244R0_Dependent_Executor_for_a_Unified_Executors_Proposal.html

PANDOC_FLAGS = -f markdown \
	       --variable urlcolor=cyan

CITEPROC= --filter pandoc-citeproc \
	  --csl=acm-sig-proceedings-long-author-list.csl

explanatory_pdf: explanatory.md explanatory_header.tex explanatory_metadata.yaml
	pandoc $(PANDOC_FLAGS) $(CITEPROC) -H explanatory_header.tex explanatory_metadata.yaml explanatory.md -o P0761R3_Executors_Design_Document.pdf

explanatory_html: explanatory.md explanatory_header.tex explanatory_metadata.yaml
	pandoc $(PANDOC_FLAGS) $(CITEPROC) -H explanatory_header.tex explanatory_metadata.yaml explanatory.md -o P0761R3_Executors_Design_Document.html

clean:
	rm -f *0443*.pdf *0443*.html *0761*.pdf *0761*.html *1244*.pdf *1244*.html


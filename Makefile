pdf: README.md
	pandoc README.md -o README.pdf -H header.tex --number-sections -o P0443R1_A_Unified_Executors_Proposal.pdf

html: README.md
	pandoc README.md -o README.html --number-sections -o P0443R1_A_Unified_Executors_Proposal.html

clean:
	rm P0443R1*


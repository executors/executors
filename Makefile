pdf: README.md
	pandoc README.md -o README.pdf -H header.tex --number-sections -o D0443R2_A_Unified_Executors_Proposal.pdf

html: README.md
	pandoc README.md -o README.html --number-sections -o D0443R2_A_Unified_Executors_Proposal.html

clean:
	rm D0443R2*


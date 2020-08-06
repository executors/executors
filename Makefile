all: wording design

wording: front_matter.md design.md wording.md appendices.md P0443-syntax.xml Makefile
	pandoc --standalone -Vlinkcolor:blue --syntax-definition=P0443-syntax.xml --highlight-style=haddock --number-sections front_matter.md design.md wording.md appendices.md -o P0443R14_A_Unified_Executors_Proposal.html

design: design.md appendices.md Makefile
	pandoc -Vgeometry:margin=0.75in --standalone -Vlinkcolor:blue --syntax-definition=P0443-syntax.xml --highlight-style=haddock --number-sections design.md appendices.md -o design.pdf

clean:
	rm -f *.html *.pdf


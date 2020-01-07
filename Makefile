all: wording

wording: wording.md wording_front_matter.md wording_back_matter.md P0443-syntax.xml Makefile
	pandoc --standalone -Vlinkcolor:blue --syntax-definition=P0443-syntax.xml --highlight-style=haddock wording_front_matter.md wording.md wording_back_matter.md -o wording.html --number-sections -o P0443R12_A_Unified_Executors_Proposal.html

clean:
	rm -f *0443*.pdf *0443*.html *0761*.pdf *0761*.html *1244*.pdf *1244*.html


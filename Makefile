pdf: README.md
	pandoc README.md -o README.pdf -H header.tex --number-sections -o executors.issaquah_2016.pdf

html: README.md
	pandoc README.md -o README.html --number-sections -o executors.issaquah_2016.html

clean:
	rm executors.issaquah_2016.pdf


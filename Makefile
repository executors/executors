direct: README.md
	pandoc README.md -o README.pdf --number-sections -o executors.issaquah_2016.pdf

clean:
	rm executors.issaquah_2016.pdf


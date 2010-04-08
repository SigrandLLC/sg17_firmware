.PHONY: all clean

all: book.html

%.html: %.xml
	sgmltools --jade-opt='-E 0' $<

clean:
	rm -f *.html


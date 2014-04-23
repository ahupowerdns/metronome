.PHONY: all clean check

all:
	doxygen yahttp.cfg

check:
	true

distdir: all
	cp -r html $(distdir)

clean:
	rm -rf html

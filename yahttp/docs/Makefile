.PHONY: all clean check distclean

all:
	doxygen yahttp.cfg

check:
	true

distdir: all
	cp -r html $(distdir)

clean:
	rm -rf html

distclean: clean

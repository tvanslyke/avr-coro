
release/libtimcoro.a: build_library
	cp src/libtimcoro.a release/libtimcoro.a

build_library:
	cd src && $(MAKE)

clean:
	cd src/ && $(MAKE) clean
	rm release/libtimcoro.a


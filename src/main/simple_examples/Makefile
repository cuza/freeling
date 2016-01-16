
# Makefile to build the simple examples of FreeLing usage.
# If your installation paths are not /usr/local, call make with:
#
# make FREELINGDIR=/my/freeling/dir BOOSTINCLUDEDIR=/my/boost/include BOOSTLIBDIR=/my/boost/lib


# directories where freeling and boost are installed
FREELINGDIR=/usr/local
BOOSTINCLUDEDIR=/usr/local/include
BOOSTLIBDIR=/usr/local/lib

# paths for compilation commands below
PATHS=-L$(FREELINGDIR)/lib -I$(FREELINGDIR)/include -L$(BOOSTLIBDIR) -I$(BOOSTINCLUDEDIR)

all:	fl1_morph fl2_tagger fl3_chunker fl4_dependences ident coreferences sample kbest_tagger alts

fl1_morph: fl1_morph.cc
	g++ -o fl1_morph fl1_morph.cc -lfreeling $(PATHS)

fl2_tagger: fl2_tagger.cc
	g++ -o fl2_tagger fl2_tagger.cc -lfreeling $(PATHS)

fl3_chunker: fl3_chunker.cc
	g++ -o fl3_chunker fl3_chunker.cc -lfreeling $(PATHS)

fl4_dependences: fl4_dependences.cc
	g++ -o fl4_dependences fl4_dependences.cc -lfreeling $(PATHS)

ident: ident.cc
	g++ -o ident ident.cc -lfreeling $(PATHS)

coreferences: coreferences.cc 
	g++ -o coreferences coreferences.cc -lfreeling $(PATHS)

sample: sample.cc
	g++ -o sample sample.cc -lfreeling $(PATHS)

kbest_tagger: kbest_tagger.cc
	g++ -o kbest_tagger kbest_tagger.cc -lfreeling $(PATHS)

alts: alts.cc
	g++ -o alts alts.cc -lfreeling $(PATHS)

clean:
	rm -f fl1_morph fl2_tagger fl3_chunker fl4_dependences ident coreferences sample kbest_tagger alts

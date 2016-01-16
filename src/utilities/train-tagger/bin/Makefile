
# Adjust this path to your FreeLing installation
FLDIR=/usr/local

all: update-probs change-corpus-dict

update-probs: update-probs.cc
	g++ -o update-probs update-probs.cc -lfreeling -I$(FLDIR)/include -L$(FLDIR)/lib -std=gnu++0x

change-corpus-dict: change-corpus-dict.cc
	g++ -o change-corpus-dict change-corpus-dict.cc -lfreeling -I$(FLDIR)/include -L$(FLDIR)/lib -std=gnu++0x

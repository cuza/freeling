languages = as ca cs cy en es fr gl it pt ru
variants = es/old-es

pkgdata_DATA =

asdatadir = $(pkgdatadir)/as
asdata_DATA = as/*.*
asnecdatadir = $(pkgdatadir)/as/nec
asnecdata_DATA = as/nec/*
aschunkdir = $(pkgdatadir)/as/chunker
aschunk_DATA = as/chunker/*
asdepdatadir = $(pkgdatadir)/as/dep
asdepdata_DATA = as/dep/*

cadatadir = $(pkgdatadir)/ca
cadata_DATA = ca/*.*
canercdatadir = $(pkgdatadir)/ca/nerc
canecdatadir = $(pkgdatadir)/ca/nerc/nec
canerdatadir = $(pkgdatadir)/ca/nerc/ner
cadnercdatadir = $(pkgdatadir)/ca/nerc/data
canecdata_DATA = ca/nerc/nec/*
canerdata_DATA = ca/nerc/ner/*
cadnercdata_DATA = ca/nerc/data/*
cachunkdir = $(pkgdatadir)/ca/chunker
cachunk_DATA = ca/chunker/*
cadepdatadir = $(pkgdatadir)/ca/dep
cadepdata_DATA = ca/dep/*

csdatadir = $(pkgdatadir)/cs
csdata_DATA = cs/*.*

cydatadir = $(pkgdatadir)/cy
cydata_DATA = cy/*.*
cynecdatadir = $(pkgdatadir)/cy/nec
cynecdata_DATA = cy/nec/*

endatadir = $(pkgdatadir)/en
endata_DATA = en/*.*
ennercdatadir = $(pkgdatadir)/en/nerc
ennecdatadir = $(pkgdatadir)/en/nerc/nec
ennerdatadir = $(pkgdatadir)/en/nerc/ner
endnercdatadir = $(pkgdatadir)/en/nerc/data
ennecdata_DATA = en/nerc/nec/*
ennerdata_DATA = en/nerc/ner/*
endnercdata_DATA = en/nerc/data/*
enchunkdir = $(pkgdatadir)/en/chunker
enchunk_DATA = en/chunker/*
endepdatadir = $(pkgdatadir)/en/dep
endepdata_DATA = en/dep/*

esdatadir = $(pkgdatadir)/es
esdata_DATA = es/*.*
esnercdatadir = $(pkgdatadir)/es/nerc
esnecdatadir = $(pkgdatadir)/es/nerc/nec
esnerdatadir = $(pkgdatadir)/es/nerc/ner
esdnercdatadir = $(pkgdatadir)/es/nerc/data
esnecdata_DATA = es/nerc/nec/*
esnerdata_DATA = es/nerc/ner/*
esdnercdata_DATA = es/nerc/data/*
eschunkdir = $(pkgdatadir)/es/chunker
eschunk_DATA = es/chunker/*
esdepdatadir = $(pkgdatadir)/es/dep
esdepdata_DATA = es/dep/*
escorefdatadir = $(pkgdatadir)/es/coref
escorefdata_DATA = es/coref/*
oldesdatadir = $(pkgdatadir)/es/old-es
oldesdata_DATA = es/old-es/*.*

frdatadir = $(pkgdatadir)/fr
frdata_DATA = fr/*.*

gldatadir = $(pkgdatadir)/gl
gldata_DATA = gl/*.*
glnercdatadir = $(pkgdatadir)/gl/nerc
glnecdatadir = $(pkgdatadir)/gl/nerc/nec
glnerdatadir = $(pkgdatadir)/gl/nerc/ner
gldnercdatadir = $(pkgdatadir)/gl/nerc/data
glnecdata_DATA = gl/nerc/nec/*
glnerdata_DATA = gl/nerc/ner/*
gldnercdata_DATA = gl/nerc/data/*
glchunkdir = $(pkgdatadir)/gl/chunker
glchunk_DATA = gl/chunker/*
gldepdatadir = $(pkgdatadir)/gl/dep
gldepdata_DATA = gl/dep/*

itdatadir = $(pkgdatadir)/it
itdata_DATA = it/*.*
itnecdatadir = $(pkgdatadir)/it/nec
itnecdata_DATA = it/nec/*

ptdatadir = $(pkgdatadir)/pt
ptdata_DATA = pt/*.*
ptnercdatadir = $(pkgdatadir)/pt/nerc
ptnecdatadir = $(pkgdatadir)/pt/nerc/nec
ptnerdatadir = $(pkgdatadir)/pt/nerc/ner
ptdnercdatadir = $(pkgdatadir)/pt/nerc/data
ptnecdata_DATA = pt/nerc/nec/*
ptnerdata_DATA = pt/nerc/ner/*
ptdnercdata_DATA = pt/nerc/data/*
ptchunkdir = $(pkgdatadir)/pt/chunker
ptchunk_DATA = pt/chunker/*

rudatadir = $(pkgdatadir)/ru
rudata_DATA = ru/*.*

commondatadir = $(pkgdatadir)/common
commondata_DATA = common/*.*
commonnecdatadir = $(pkgdatadir)/common/nec
commonnecdata_DATA = common/nec/*
commonalternativesdatadir = $(pkgdatadir)/common/alternatives
commonalternativesdata_DATA = common/alternatives/*
commonlangidentdatadir = $(pkgdatadir)/common/lang_ident
commonlangidentdata_DATA = common/lang_ident/*

configdatadir = $(pkgdatadir)/config
configdata_DATA = config/*

install-data-hook:
	-@ldconfig &> /dev/null || echo "Could not run 'ldconfig'. If you get installation errors below, execute 'ldconfig' as root and try 'make install' again.";
	@echo "Indexing language dictionaries. This will take some minutes.";
	@for lg in $(languages); do \
	   echo "Installing files for language '"$$lg"'. Please wait."; \
	   echo "  - Creating "$$lg" dictionary..."; \
	   $(top_builddir)/src/utilities/dicc-management/bin/build-dict.sh $(top_builddir)/data/$$lg/dictionary/header $(top_builddir)/data/$$lg/dictionary/entries/* > $(DESTDIR)/$(pkgdatadir)/$$lg/dicc.src; \
	   for sublg in $(variants); do \
	      if (echo $$sublg | grep -q $$lg/); then \
		 echo "  - Creating "$$sublg" variant dictionary..."; \
		 $(top_builddir)/src/utilities/dicc-management/bin/build-dict.sh $(top_builddir)/data/$$sublg/dictionary/header $(top_builddir)/data/$$lg/dictionary/entries/* $(top_builddir)/data/$$sublg/dictionary/entries/* > $(DESTDIR)/$(pkgdatadir)/$$sublg/dicc.src; \
	      fi;\
	   done; \
	   if (test -f "$(DESTDIR)/$(pkgdatadir)/$$lg/alternatives-phon.dat" ); then \
	      echo "  - Creating "$$lg" phonetic dictionary..."; \
              phRul=`cat $(DESTDIR)/$(pkgdatadir)/$$lg/alternatives-phon.dat | grep PhoneticRules | cut -d' ' -f2`; \
              phDic=`cat $(DESTDIR)/$(pkgdatadir)/$$lg/alternatives-phon.dat | grep PhoneticDictionary | cut -d' ' -f2`; \
	      cat $(DESTDIR)/$(pkgdatadir)/$$lg/dicc.src | LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(DESTDIR)/$(libdir) $(DESTDIR)/$(bindir)/dicc2phon $(DESTDIR)/$(pkgdatadir)/$$lg/$$phRul > $(DESTDIR)/$(pkgdatadir)/$$lg/$$phDic; \
           fi; \
	done

uninstall-hook:
	@rm -rf $(DESTDIR)/$(pkgdatadir)

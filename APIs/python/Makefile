
# You may need to change these paths to match your installation.
# Alternatively, you can call 'make' overriding variable values, E.g.:
#     make FREELINGDIR=/my/freeling/dir PYTHONDIR=/my/python/dir

FREELINGDIR = /usr/local
PYTHONDIR = /usr/include/python3.2

_freeling.so: freeling_pythonAPI.cxx
	g++ -shared -o _freeling.so freeling_pythonAPI.cxx -lfreeling -I$(FREELINGDIR)/include -L$(FREELINGDIR)/lib -I$(PYTHONDIR) -fPIC
#       Mac OSX users:  Uncomment command below (and comment the one above) 
#	g++ -dynamiclib -o _freeling.so freeling_pythonAPI.cxx -lpython3.2 -lfreeling -I$(FREELINGDIR)/include -L$(FREELINGDIR)/lib -I$(PYTHONDIR) $(CPPFLAGS) $(LDFLAGS) -fPIC

freeling_pythonAPI.cxx: freeling_pythonAPI.i ../common/freeling.i
	swig -python -py3 -c++ -o freeling_pythonAPI.cxx freeling_pythonAPI.i

clean:
	rm -rf __pycache__ freeling_pythonAPI.cxx _freeling.so freeling.py*

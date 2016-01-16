
# You may need to change these paths to match your installation
# Alternatively, you can call 'make' overriding variable values, E.g.:
#     make FREELINGDIR=/my/freeling/dir PERLDIR=/my/ruby/dir

FREELINGDIR = /usr/local
RUBYDIR = /usr/lib/ruby/1.8/x86_64-linux

freeling.so: freeling_rubyAPI.cxx
	g++ -shared -o freeling.so freeling_rubyAPI.cxx -lfreeling -I$(FREELINGDIR)/include -L$(FREELINGDIR)/lib -I$(RUBYDIR) -fPIC

freeling_rubyAPI.cxx: freeling_rubyAPI.i
	swig -ruby -c++ -o freeling_rubyAPI.cxx freeling_rubyAPI.i

clean:
	rm -f freeling_rubyAPI.cxx freeling.so
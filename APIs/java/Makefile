
# Change these paths to match your installation
# Alternatively, you can call 'make' overriding variable values, E.g.:
#     make FREELINGDIR=/my/freeling/dir SWIGDIR=/my/swig/dir JAVADIR=/my/java/dir

FREELINGDIR = /opt/local
SWIGDIR = /usr/local/Cellar/swig/3.0.7/share/swig/3.0.7
JAVADIR = /Library/Java/JavaVirtualMachines/1.8.0.jdk/Contents/Home

# you may change the package name if you want
PACKAGE = edu/upc/freeling
JPACKAGE = edu.upc.freeling

# Change your compiler, if necessary.
GCC=g++

all: freeling_javaAPI.cxx libfreeling_javaAPI.dylib

.PHONY: all

# Compile the C++ API
libfreeling_javaAPI.dylib: freeling_javaAPI.cxx
	$(GCC) -dynamiclib -o libfreeling_javaAPI.dylib freeling_javaAPI.cxx -lfreeling -lboost_system-mt -L$(FREELINGDIR)/lib -I$(FREELINGDIR)/include -I$(JAVADIR)/include -I$(JAVADIR)/include/darwin -fPIC
	$(JAVADIR)/bin/javac $(PACKAGE)/*.java
	$(JAVADIR)/bin/jar -cf freeling.jar $(PACKAGE)

# Build the Java JNI wrapper classes.
freeling_javaAPI.cxx: freeling_javaAPI.i ../common/freeling.i
	rm -rf $(PACKAGE)
	mkdir -p $(PACKAGE)
	swig -java -c++ -package $(JPACKAGE) -outdir $(PACKAGE) -o freeling_javaAPI.cxx -I$(SWIGDIR)/java -I$(SWIGDIR)/std -I$(SWIGDIR) freeling_javaAPI.i

clean:
	rm -rf *.jar *.dylib *.cxx *.class edu

install:
	mkdir -p ~/Library/Java/Extensions
	cp libfreeling_javaAPI.dylib ~/Library/Java/Extensions

uninstall:
	rm ~/Library/Java/Extensions/libfreeling_javaAPI.dylib
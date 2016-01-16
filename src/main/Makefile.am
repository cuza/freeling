EXTRA_DIST = omlet sample_analyzer simple_examples README
AM_CPPFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/libfreeling/corrector

bin_PROGRAMS = analyzer analyzer_client threaded_analyzer

include $(top_srcdir)/src/automake_options.am

# threaded analyzer sample program
threaded_analyzer_SOURCES = sample_analyzer/threaded_analyzer.cc sample_analyzer/output.cc sample_analyzer/config.h sample_analyzer/output.h  sample_analyzer/threaded_processor.h
threaded_analyzer_LDADD = -lfreeling $(FREELING_DEPS) -lboost_program_options$(MT) -lboost_system$(MT) -lboost_thread$(MT) 
threaded_analyzer_LDFLAGS = -L$(top_srcdir)/src/libfreeling

# analyzer sample program
analyzer_SOURCES = sample_analyzer/main.cc sample_analyzer/analyzer.cc sample_analyzer/output.cc sample_analyzer/config.h sample_analyzer/analyzer.h sample_analyzer/output.h sample_analyzer/socket.h sample_analyzer/stats.h
analyzer_LDADD = -lfreeling $(FREELING_DEPS) -lboost_program_options$(MT) -lpthread -lboost_system$(MT)
analyzer_LDFLAGS = -L$(top_srcdir)/src/libfreeling

# sample client for analyzer in server mode
analyzer_client_SOURCES = sample_analyzer/analyzer_client.cc sample_analyzer/socket.h

install-exec-hook:
	cp sample_analyzer/analyze sample_analyzer/fl_initialize $(DESTDIR)/$(bindir)
	chmod +x $(DESTDIR)/$(bindir)/analyze

uninstall-hook:
	@rm -rf $(DESTDIR)/$(bindir)/analyze $(DESTDIR)/$(bindir)/fl_initialize

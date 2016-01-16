EXTRA_DIST = windll.cc

AM_CPPFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/include/freeling/foma

lib_LTLIBRARIES = libfreeling.la

include $(top_srcdir)/src/automake_options.am

libfreeling_la_SOURCES = version.cc util.cc regexp.cc traces.cc language.cc configfile.cc tokenizer.cc splitter.cc processor.cc RE_map.cc dictionary.cc suffixes.cc accents/accents.cc accents/accents_default.cc accents/accents_es.cc accents/accents_gl.cc prefTree.cc database.cc punts.cc automat.cc numbers/numbers.cc numbers/numbers_default.cc numbers/numbers_ca.cc numbers/numbers_cs.cc numbers/numbers_en.cc numbers/numbers_es.cc numbers/numbers_gl.cc numbers/numbers_pt.cc numbers/numbers_ru.cc numbers/numbers_it.cc dates/dates.cc dates/dates_default.cc dates/dates_ca.cc dates/dates_gl.cc dates/dates_pt.cc dates/dates_en.cc dates/dates_es.cc dates/dates_ru.cc locutions.cc ner.cc ner_module.cc np.cc bioner.cc quantities/quantities.cc quantities/quantities_default.cc quantities/quantities_ca.cc quantities/quantities_en.cc quantities/quantities_es.cc quantities/quantities_gl.cc quantities/quantities_pt.cc quantities/quantities_ru.cc probabilities.cc maco.cc maco_options.cc phonetics.cc tagset.cc tagger.cc hmm_tagger.cc lexer.cc relax_tagger/relax_tagger.cc relax_tagger/relax.cc relax_tagger/constraint_grammar.cc nec.cc senses.cc semdb.cc chart_parser/chart_parser.cc chart_parser/chart.cc chart_parser/grammar.cc dep_rules.cc dependency_parser.cc dependencies.cc ukb.cc csr_kb.cc foma_FSM.cc alternatives.cc lang_ident/idioma.cc lang_ident/lang_ident.cc fex/fex_rule.cc fex/fex_lexicon.cc fex/fex.cc fex/nerc_features.cc omlet/classifier.cc omlet/adaboost.cc omlet/dataset.cc omlet/example.cc omlet/weakrule.cc omlet/viterbi.cc omlet/svm.cc omlet/libsvm.cc coref/coref.cc coref/coref_fex.cc foma/int_stack.c foma/define.c foma/determinize.c foma/apply.c foma/rewrite.c foma/lexcread.c foma/topsort.c foma/flags.c foma/minimize.c foma/reverse.c foma/extract.c foma/sigma.c foma/io.c foma/structures.c foma/constructions.c foma/coaccessible.c foma/utf8.c foma/spelling.c foma/dynarray.c foma/mem.c foma/stringhash.c foma/trie.c foma/lex.lexc.c foma/lex.yy.c foma/lex.cmatrix.c foma/regex-tab.c

libfreeling_la_LIBADD = $(FREELING_DEPS) -lz
libfreeling_la_LDFLAGS = -release $(PACKAGE_VERSION)

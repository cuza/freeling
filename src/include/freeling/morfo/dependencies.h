//////////////////////////////////////////////////////////////////
//
//    FreeLing - Open Source Language Analyzers
//
//    Copyright (C) 2004   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU General Public
//    License as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    General Public License for more details.
//
//    You should have received a copy of the GNU General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
//             despatx C6.212 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

#ifndef _DEP_TXALA
#define _DEP_TXALA

#include <string>
#include <map>
#include <set>
#include <vector>

#include "freeling/windll.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/processor.h"
#include "freeling/morfo/semdb.h"
#include "freeling/morfo/dep_rules.h"
#include "freeling/morfo/dependency_parser.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  /// Store parsing status information
  ////////////////////////////////////////////////////////////////

  class dep_txala_status : public processor_status {
  public:
    /// precomputed last node matching the "last_left/right" condition
    /// for a certain rule number
    std::map<std::wstring,parse_tree::iterator> last;

    /// set of active flags, which control applicability of rules
    std::set<std::wstring> active_flags;
  };

  ////////////////////////////////////////////////////////////////
  ///  The class completer implements a parse tree completer,
  /// which given a partial parse tree (chunker output), completes
  /// the full parse according to some grammar rules.
  ////////////////////////////////////////////////////////////////

  class completer {
  private:
    /// set of rules, indexed by labels of nodes
    std::map<std::pair<std::wstring,std::wstring>,std::list<completerRule> > chgram;
    // semantic classes for words, declared in CLASS section
    std::set<std::wstring> wordclasses;  // items are class#lemma

    /// retrieve rule from grammar
    completerRule find_grammar_rule(const std::vector<parse_tree *> &, const size_t, dep_txala_status*) const;
    /// apply a completion rule
    parse_tree * applyRule(const completerRule &, int, parse_tree*, parse_tree*, dep_txala_status*) const;
    /// check if the extra lemma/form/class conditions are satisfied
    bool match_condition(parse_tree::iterator, const matching_condition &) const;
    /// check if the current context matches the given rule
    bool matching_context(const std::vector<parse_tree *> &, const size_t, const completerRule &) const;
    /// check if the operation is executable (for last_left/last_right cases)
    bool matching_operation(const std::vector<parse_tree *> &, const size_t, const completerRule &, dep_txala_status*) const;
    /// check left or right context
    bool match_side(const int, const std::vector<parse_tree *> &, const size_t, const std::vector<matching_condition> &) const;
    /// Separate extra lemma/form/class conditions from the chunk label
    void extract_conds(std::wstring &, matching_condition &) const;
    /// Find out if currently active flags enable the given rule
    bool enabled_rule(const completerRule &, dep_txala_status*) const;

  public:  
    /// Constructor. Load a tree-completion grammar 
    completer(const std::wstring &);
    /// find best completions for given parse tree
    parse_tree complete(parse_tree &, const std::wstring &, dep_txala_status*) const;

    /// auxiliary to load CLASS section of config file, used both by completer and labeler.
    static void load_classes(const std::wstring &, const std::wstring &, 
                             const std::wstring &, std::set<std::wstring> &);

  };


  ////////////////////////////////////////////////////////////////////////
  ///
  /// depLabeler is class to set labels into a dependency tree
  ///
  ///////////////////////////////////////////////////////////////////////

  class depLabeler {

  private:
    // set of rules
    std::map<std::wstring, std::list<ruleLabeler> > rules;
    // "unique" labels
    std::set<std::wstring> unique;
    // semantic database to check for semantic conditions in rules
    semanticDB * semdb;
    // parse a condition and create checkers.
    rule_expression* build_expression(const std::wstring &) const;
    // semantic classes for words, declared in CLASS section
    std::set<std::wstring> wordclasses;  // items are class#lemma

  public:
    /// Constructor. create dependency parser
    depLabeler(const std::wstring &);
    /// Destructor
    ~depLabeler();
    /// Label nodes in a dependency tree. (Initial call)
    void label(dep_tree*) const;
    /// Label nodes in a dependency tree. (recursive)
    void label(dep_tree*, dep_tree::iterator) const;
  };



  ///////////////////////////////////////////////////////////////////////
  ///
  /// dependencyMaker is a class for obtaining a dependency tree from chunks.
  ///  this implementation uses two subclasses:
  /// completer: to complete the chunk analysis in a full parse tree
  /// depLabeler: to set the labels once the class has build a dependency tree
  ///
  ///////////////////////////////////////////////////////////////////////

  class WINDLL dep_txala : public dependency_parser {

  private:
    /// tree completer  
    completer comp; 
    /// dependency labeler
    depLabeler labeler;
    // Root symbol used by the chunk parser when the tree is not complete.
    std::wstring start;
    /// compute dependency tree
    dep_tree* dependencies(parse_tree::iterator, parse_tree::iterator) const;

  public:   
    /// constructor
    dep_txala(const std::wstring &, const std::wstring &);

    /// analyze given sentences
    void analyze(sentence &) const;

    /// inherit other methods
    using processor::analyze;
  };

} // namespace

#endif


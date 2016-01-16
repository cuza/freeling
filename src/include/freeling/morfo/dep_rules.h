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

#ifndef _DEPRULES
#define _DEPRULES

#include <sstream>
#include <iostream>
#include <set>
#include <list>

#include "freeling/regexp.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/semdb.h"

namespace freeling {

  ///////////////////////////////////////////////////////////////
  ///  The class matching attrib stores attributes for maching condition
  ///////////////////////////////////////////////////////////////

  class matching_attrib {
  public:
    /// condition type ("<": lemma, "(": form, "[": class, "{": PoS)
    std::wstring type;
    /// string to match (for lemma, form and class)
    std::wstring value;
    /// regexp to match (for PoS)
    freeling::regexp re;

    /// constructor
    matching_attrib();
    /// copy
    matching_attrib(const matching_attrib &);
    /// destructor
    ~matching_attrib();     
  };

  ////////////////////////////////////////////////////////////////
  ///  The class matching condition stores a condition used
  /// in a MATCHING operation of a completer rule. Also used
  /// in context and chunk expressions in completer rules.
  ////////////////////////////////////////////////////////////////

  class matching_condition {
  public:
    bool neg;
    std::wstring label;
    std::list<matching_attrib> attrs;
  };

  ////////////////////////////////////////////////////////////////
  ///
  ///  The class completerRule stores rules used by the
  /// completer of parse trees
  ///
  ////////////////////////////////////////////////////////////////

  class completerRule {
 
  public:
    /// line in the file where rule was, useful to trace and issue errors.
    /// Used also as rule id when storing last_left/right matches in status
    int line;

    /// chunk labels to which the rule is applied
    std::wstring leftChk;
    std::wstring rightChk;
    /// extra conditions on the chunks (pos, lemma, form, class)
    matching_condition leftConds;
    matching_condition rightConds;
    /// new label/s (if any) for the nodes after the operation. 
    std::wstring newNode1;
    std::wstring newNode2;
    /// condition for MATCHING operation.
    matching_condition matchingCond;
    /// operation to perform
    std::wstring operation;
    /// context (if any) required to apply the rule
    std::vector<matching_condition> leftContext;
    std::vector<matching_condition> rightContext;
    /// whether the context is negated
    bool context_neg;
    /// priority of the rule
    int weight;

    /// flags that enable the rule to be applied
    std::set<std::wstring> enabling_flags;
    /// flags to toggle on after applying the rule
    std::set<std::wstring> flags_toggle_on;
    /// flags to toggle off after applying the rule
    std::set<std::wstring> flags_toggle_off;

   
    /// constructors
    completerRule();
    completerRule(const std::wstring &, const std::wstring &, const std::wstring&);
    completerRule( const completerRule &);
    /// assignment
    completerRule & operator=( const completerRule &);
   
    /// Comparison. The more weight the higher priority 
    int operator<(const completerRule & a ) const;
  };


  ////////////////////////////////////////////////////////////////
  ///
  /// The class rule_expression is an abstract class (interface)
  /// for building dynamic restriction on a ruleLabeler 
  ///  which are used by class depLabeler
  ///
  ////////////////////////////////////////////////////////////////
  class rule_expression {

  private:
    void parse_node_ref(std::wstring, dep_tree::iterator, std::list<dep_tree::iterator> &) const;

  protected:
    // node the expression has to be checked against (p/d)
    std::wstring node;
    // set of values (if any) to check against.
    std::set<std::wstring> valueList;
    // obtain the iterator to the nodes to be checked 
    // and the operation AND/OR to perform
    bool nodes_to_check(dep_tree::iterator, dep_tree::iterator, std::list<dep_tree::iterator> &) const;
    // virtual, evaluate expression
    virtual bool eval(dep_tree::iterator) const;

  public:
    // constructors and destructors
    rule_expression();
    rule_expression(const std::wstring &,const std::wstring &);
    virtual ~rule_expression() {}
    // search a value in expression list
    bool find(const std::wstring &) const;
    bool find_match(const std::wstring &) const;
    bool match(const std::wstring &) const;
    bool find_any(const std::list<std::wstring> &) const;
    bool find_any_match(const std::list<std::wstring> &) const;
    // virtual, evaluate expression
    virtual bool check(dep_tree::iterator, dep_tree::iterator) const;
  };


  /////////////////////////////////////////////////////////////////
  ///
  /// The following classes are implementations of different constraints
  ///
  /// check_and:  and of basic constraints
  /// check_not:  negation
  /// check_side: side (left/rigth) of descendant wrt ancestor 
  /// check_lemma: check if lemma of given node is in list of lemmas (separator character is |)
  /// check_category: check PoS category of given node
  /// check_wordclass: check if lemma of given node belongs to a word class
  /// check_tonto: Check if given node has a top ontology property
  ///
  /////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////
  /// And of basic contraints
  ////////////////////////////////////////////////////////////////

  class check_and : public rule_expression {
  private:
    std::list<rule_expression *> check_list;
  public:
    void add(rule_expression *);
    bool check(dep_tree::iterator, dep_tree::iterator) const;
  };


  ////////////////////////////////////////////////////////////////
  /// negation
  ////////////////////////////////////////////////////////////////

  class check_not : public rule_expression {
  private:
    rule_expression * check_op;
  public:
    check_not(rule_expression *);
    bool check(dep_tree::iterator, dep_tree::iterator) const;
  };


  ////////////////////////////////////////////////////////////////
  ///  side of descendant respect to ancestor (or viceversa)
  ////////////////////////////////////////////////////////////////

  class check_side : public rule_expression {
  public:
    check_side(const std::wstring &,const std::wstring &);
    bool check(dep_tree::iterator, dep_tree::iterator) const;
  };


  ////////////////////////////////////////////////////////////////
  /// find lemma in list of lemmas (separator character is |)
  ////////////////////////////////////////////////////////////////

  class check_lemma : public rule_expression {
  public:
    check_lemma(const std::wstring &,const std::wstring &);
    bool eval(dep_tree::iterator) const;
  };

  ////////////////////////////////////////////////////////////////
  /// match pos against regexp
  ////////////////////////////////////////////////////////////////

  class check_pos : public rule_expression {
  public:
    check_pos(const std::wstring &,const std::wstring &);
    bool eval(dep_tree::iterator) const;
  };


  ////////////////////////////////////////////////////////////////
  /// check category category
  ////////////////////////////////////////////////////////////////

  class check_category :public rule_expression {
  public:
    check_category(const std::wstring &,const std::wstring &);
    bool eval(dep_tree::iterator) const;
  };


  ////////////////////////////////////////////////////////////////
  /// lemma belongs to a class
  ////////////////////////////////////////////////////////////////

  class check_wordclass : public rule_expression { 
  private: 
    const std::set<std::wstring> *wordclasses; // word class set from labeler we belong to.
  public:
    check_wordclass(const std::wstring &, const std::wstring &, const std::set<std::wstring> *);
    bool eval(dep_tree::iterator) const;
  };

  ////////////////////////////////////////////////////////////////
  /// lemma has given top ontology class
  ////////////////////////////////////////////////////////////////

  class check_tonto : public rule_expression { 
  private: 
    semanticDB &semdb;
  public:    
    check_tonto(semanticDB &, const std::wstring &, const std::wstring &);
    bool eval(dep_tree::iterator) const;
  };

  ////////////////////////////////////////////////////////////////
  /// lemma has given WN semantic file
  ////////////////////////////////////////////////////////////////

  class check_semfile : public rule_expression { 
  private: 
    semanticDB &semdb;
  public:    
    check_semfile(semanticDB &, const std::wstring &, const std::wstring &);
    bool eval(dep_tree::iterator) const;
  };

  ////////////////////////////////////////////////////////////////
  /// lemma has given synonym
  ////////////////////////////////////////////////////////////////

  class check_synon : public rule_expression { 
  private: 
    semanticDB &semdb;
  public:    
    check_synon(semanticDB &, const std::wstring &, const std::wstring &);
    bool eval(dep_tree::iterator) const;
  };

  ////////////////////////////////////////////////////////////////
  /// lemma or any ancestor has given synonym
  ////////////////////////////////////////////////////////////////

  class check_asynon : public rule_expression { 
  private: 
    semanticDB &semdb;
  public:    
    check_asynon(semanticDB &, const std::wstring &, const std::wstring &);
    bool eval(dep_tree::iterator) const;
  };


  ////////////////////////////////////////////////////////////////
  /// ruleLabeler is an auxiliary class for the depLabeler
  ////////////////////////////////////////////////////////////////

  class ruleLabeler {

  public:
    std::wstring label;
    rule_expression * re;
    std::wstring ancestorLabel;
    /// line in the file where rule was, useful to trace and issue errors
    int line;

    ruleLabeler(void);
    ruleLabeler(const std::wstring &, rule_expression *);
    bool check(dep_tree::iterator, dep_tree::iterator) const;
  };

} // namespace

#endif

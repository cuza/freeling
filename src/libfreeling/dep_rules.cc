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

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/dep_rules.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"DEPENDENCIES"
#define MOD_TRACECODE DEP_TRACE

  //---------- Class matching_attrib ----------------------------------

  ////////////////////////////////////////////////////////////////
  /// constructor
  ////////////////////////////////////////////////////////////////

  matching_attrib::matching_attrib() : re(L"") {};

  ////////////////////////////////////////////////////////////////
  /// copy
  ////////////////////////////////////////////////////////////////

  matching_attrib::matching_attrib(const matching_attrib &ma) : re(ma.re) {
    type=ma.type; value=ma.value;
  }

  ////////////////////////////////////////////////////////////////
  /// constructor
  ////////////////////////////////////////////////////////////////

  matching_attrib::~matching_attrib() {};


  //---------- Class completerRule ----------------------------------

  ////////////////////////////////////////////////////////////////
  ///  Constructor
  ////////////////////////////////////////////////////////////////

  completerRule::completerRule() {
    weight=0;
  }

  ////////////////////////////////////////////////////////////////
  ///  Constructor
  ////////////////////////////////////////////////////////////////

  completerRule::completerRule(const wstring &pnewNode1, const wstring &pnewNode2, const wstring &poperation) {
    operation=poperation;
    newNode1=pnewNode1;
    newNode2=pnewNode2;
    line=0;
    context_neg=false;
    weight=0;
  }


  ////////////////////////////////////////////////////////////////
  ///  Constructor
  ////////////////////////////////////////////////////////////////

  completerRule::completerRule(const completerRule & cr) {
    leftChk=cr.leftChk;     rightChk=cr.rightChk;
    leftConds=cr.leftConds; rightConds=cr.rightConds;
    newNode1=cr.newNode1;   newNode2=cr.newNode2;
    matchingCond=cr.matchingCond;
    leftContext=cr.leftContext; rightContext=cr.rightContext;
    operation=cr.operation;
    weight=cr.weight;
    context_neg=cr.context_neg;
    line=cr.line;
    enabling_flags = cr.enabling_flags;
    flags_toggle_on = cr.flags_toggle_on;
    flags_toggle_off = cr.flags_toggle_off;
  }

  ////////////////////////////////////////////////////////////////
  ///  Assignment
  ////////////////////////////////////////////////////////////////

  completerRule & completerRule::operator=( const completerRule & cr) {
    leftChk=cr.leftChk;     rightChk=cr.rightChk;
    leftConds=cr.leftConds; rightConds=cr.rightConds;
    newNode1=cr.newNode1;   newNode2=cr.newNode2;
    operation=cr.operation;
    matchingCond=cr.matchingCond;
    leftContext=cr.leftContext; rightContext=cr.rightContext;
    weight=cr.weight;
    context_neg=cr.context_neg;
    line=cr.line;
    enabling_flags = cr.enabling_flags;
    flags_toggle_on = cr.flags_toggle_on;
    flags_toggle_off = cr.flags_toggle_off;
  
    return *this;
  }

  ////////////////////////////////////////////////////////////////
  ///  Comparison. The smaller weight, the higher priority 
  ////////////////////////////////////////////////////////////////

  int completerRule::operator<(const  completerRule & a ) const  { 
    return ((weight<a.weight) && (weight>0));
  }


  //---------- Class rule_expression (and derived) -------------------

  ///////////////////////////////////////////////////////////////
  /// Constructor
  ///////////////////////////////////////////////////////////////

  rule_expression::rule_expression() {};

  ///////////////////////////////////////////////////////////////
  /// Constructor
  ///////////////////////////////////////////////////////////////

  rule_expression::rule_expression(const wstring &n,const wstring &v) {
    node=n;
    valueList=util::wstring2set(v,L"|");
  };


  ///////////////////////////////////////////////////////////////
  /// Search for a value in the list of an expression
  ///////////////////////////////////////////////////////////////

  bool rule_expression::find(const wstring &v) const {
    return (valueList.find(v) != valueList.end());
  }

  ///////////////////////////////////////////////////////////////
  /// Match the value against a RegExp
  ///////////////////////////////////////////////////////////////

  bool rule_expression::match(const wstring &v) const {
    freeling::regexp re(util::set2wstring(valueList,L"|"));
    return (re.search(v));
  }

  ///////////////////////////////////////////////////////////////
  /// Search for any value of a list in the list of an expression
  ///////////////////////////////////////////////////////////////

  bool rule_expression::find_any(const list<wstring> &ls) const {
    bool found=false;
    for (list<wstring>::const_iterator s=ls.begin(); !found && s!=ls.end(); s++)
      found=find(*s);
    return(found);
  }


  ///////////////////////////////////////////////////////////////
  /// Search for a value in the list of an expression, 
  /// taking into account wildcards
  ///////////////////////////////////////////////////////////////

  bool rule_expression::find_match(const wstring &v) const {
    for (set<wstring>::const_iterator i=valueList.begin(); i!=valueList.end(); i++) {
      TRACE(4,L"      eval "+node+L".label="+(*i)+L" (it is "+v+L")");
      // check for plain match
      if (v==(*i)) return true;  
      // not straight, check for a wildcard
      wstring::size_type p=i->find_first_of(L"*");
      if (p!=wstring::npos && i->substr(0,p)==v.substr(0,p)) 
        // there is a wildcard and matches
        return true;
    }
    return false;
  }


  ///////////////////////////////////////////////////////////////
  /// Search for any value of a list in the list of an expression,
  /// taking into account wildcards
  ///////////////////////////////////////////////////////////////

  bool rule_expression::find_any_match(const list<wstring> &ls) const {
    bool found=false;
    for (list<wstring>::const_iterator s=ls.begin(); !found && s!=ls.end(); s++)
      found=find_match(*s);
    return(found);
  }


  ///////////////////////////////////////////////////////////////
  /// Recursive disassembly of node reference string (e.g. p:sn:sajd)
  /// to get the right iterator. When (if) found, add it to given list.
  ///////////////////////////////////////////////////////////////

  void rule_expression::parse_node_ref(wstring nd, dep_tree::iterator k, list<dep_tree::iterator> &res) const {

    wstring top;
    if (nd.size()==0)
      res.push_back(k);
    else {
      TRACE(4,L"       recursing at "+nd+L", have parent "+k->info.get_link()->info.get_label());
      wstring::size_type t=nd.find(L':');
      if (t==wstring::npos) {
        top=nd;
        nd=L"";
      }
      else {
        top=nd.substr(0,t);
        nd=nd.substr(t+1);
      }
    
      TRACE(4,L"        need child "+nd);
      dep_tree::sibling_iterator j;
      for (j=k->sibling_begin(); j!=k->sibling_end(); ++j) {
        TRACE(4,L"           looking for "+top+L", found child with: "+j->info.get_link()->info.get_label());
        if (j->info.get_link()->info.get_label() == top)
          parse_node_ref(nd,j,res);
      }  
    }  
  }


  ///////////////////////////////////////////////////////////////
  /// Givent parent and daughter iterators, resolve which of them 
  /// is to be checked in this condition.
  ///////////////////////////////////////////////////////////////

  bool rule_expression::nodes_to_check(dep_tree::iterator p, dep_tree::iterator d, list<dep_tree::iterator> &res) const{

    wstring top, nd;
    wstring::size_type t=node.find(L':');
    if (t==wstring::npos) {
      top=node;
      nd=L"";
    }
    else {
      top=node.substr(0,t);
      nd=node.substr(t+1);
    }

    if (top==L"p") 
      parse_node_ref(nd,p,res);
    else if (top==L"d") 
      parse_node_ref(nd,d,res);
    else if (top==L"As" or top==L"Es") {
      // add to the list all children (except d) of the same parent (p)
      for (dep_tree::sibling_iterator s=p->sibling_begin(); s!=p->sibling_end(); ++s)
        if (s!=d)
          parse_node_ref(nd,s,res);
    }

    return (top==L"As"); // return true for AND, false for OR.
  }



  ///////////////////////////////////////////////////////////////
  /// Check wheter a rule_expression can be applied to the
  /// given pair of nodes
  ///////////////////////////////////////////////////////////////

  bool rule_expression::check(dep_tree::iterator ancestor, dep_tree::iterator descendant) const {

    list<dep_tree::iterator> ln;

    // if "which_ao"=true then "eval" of all nodes in "ln" must be joined with AND
    // if it is false, and OR must be used 
    bool which_ao = nodes_to_check(ancestor, descendant, ln);  
    if (ln.empty()) return false;

    TRACE(4,L"      found nodes to check.");

    // start with "true" for AND and "false" for OR
    bool res= which_ao; 
    // the loop goes on when res==true for AND and when res==false for OR
    for (list<dep_tree::iterator>::iterator n=ln.begin(); n!=ln.end() and (res==which_ao); n++) {
      TRACE(4,L"      checking node.");
      res=eval(*n);
    }

    return res;
  }

  ///////////////////////////////////////////////////////////////
  /// eval whether a single node matches a condition
  /// only called from check if needed. The abstract class
  /// version should never be reached.
  ///////////////////////////////////////////////////////////////

  bool rule_expression::eval(dep_tree::iterator n) const {
    return false;
  }


  //---------- Classes derived from rule_expresion ----------------------------------

  /// check_and

  void check_and::add(rule_expression * re) {check_list.push_back(re);}
  bool check_and::check(dep_tree::iterator ancestor, dep_tree::iterator descendant) const {
    TRACE(4,L"      eval AND");
    bool result=true;
    list<rule_expression *>::const_iterator ci=check_list.begin();
    while(result && ci!=check_list.end()) { 
      result=(*ci)->check(ancestor,descendant); 
      ++ci;
    }
    return result;
  }

  /// check_not

  check_not::check_not(rule_expression * re) {check_op=re;}
  bool check_not::check(dep_tree::iterator  ancestor, dep_tree::iterator  descendant) const {
    TRACE(4,L"      eval NOT");
    return (! check_op->check(ancestor,descendant));  
  }

  /// check_side

  check_side::check_side(const wstring &n,const wstring &s) : rule_expression(n,s) {
    if (valueList.size()>1 || (s!=L"right" && s!=L"left")) 
      WARNING(L"Error reading dependency rules. Invalid condition "+node+L".side="+s+L". Must be one of 'left' or 'right'.");
  };
  bool check_side::check(dep_tree::iterator ancestor, dep_tree::iterator descendant) const {
    wstring side=*valueList.begin();
    bool b=false;
    TRACE(4,L"      eval SIDE="+side+L" node="+node);
    TRACE(4,L"          d="+util::int2wstring(descendant->info.get_word().get_span_start())
          +L" p="+util::int2wstring(ancestor->info.get_word().get_span_start()) );
    if ((side==L"left" && node==L"d") || (side==L"right" && node==L"p")) 
      b= (descendant->info.get_word().get_span_start())<(ancestor->info.get_word().get_span_start());
    else if ((side==L"left" && node==L"p") || (side==L"right" && node==L"d")) 
      b = (descendant->info.get_word().get_span_start())>(ancestor->info.get_word().get_span_start());
  
    TRACE(4,L"          result = "+wstring(b?L"true":L"false"));
    return b;
  }

  /// check_wordclass

  check_wordclass::check_wordclass(const wstring &n, const wstring &c, const set<wstring> *wc) : rule_expression(n,c), wordclasses(wc) {}

  bool check_wordclass::eval (dep_tree::iterator n) const {
    TRACE(4,L"      Checking "+node+L".class="+util::set2wstring(valueList,L"|")+L" ? lemma="+ n->info.get_word().get_lemma());

    bool found=false;
    for (set<wstring>::const_iterator wclass=valueList.begin(); !found && wclass!=valueList.end(); wclass++)
      found= (wordclasses->find((*wclass)+L"#"+(n->info.get_word().get_lemma())) != wordclasses->end());
    return found;
  }

  /// check_lemma

  check_lemma::check_lemma(const wstring &n,const wstring &l) : rule_expression(n,l) {}
  bool check_lemma::eval(dep_tree::iterator n) const {
    TRACE(4,L"      eval. "+node+L".lemma "+n->info.get_word().get_lemma());
    return (find(n->info.get_word().get_lemma()));
  }

  /// check head PoS

  check_pos::check_pos(const wstring &n,const wstring &l) : rule_expression(n,l) {}
  bool check_pos::eval(dep_tree::iterator n) const {
    TRACE(4,L"      eval. "+node+L".pos "+n->info.get_word().get_tag());
    return (match(n->info.get_word().get_tag()));
  }


  /// check chunk label

  check_category::check_category(const wstring &n,const wstring &p) : rule_expression(n,p) {}
  bool check_category::eval(dep_tree::iterator n) const {
    TRACE(4,L"      eval. "+node+L".label "+n->info.get_link()->info.get_label());
    return (find_match(n->info.get_link()->info.get_label()));
  }


  /// check top-ontology

  check_tonto::check_tonto(semanticDB &db, const wstring &n, const wstring &t) : rule_expression(n,t), semdb(db) {}
  bool check_tonto::eval (dep_tree::iterator n) const {
    TRACE(4,L"      eval "+node+L".tonto "+n->info.get_link()->info.get_label());
    wstring form=n->info.get_word().get_lc_form();
    wstring lem=n->info.get_word().get_lemma();
    wstring pos=n->info.get_word().get_tag().substr(0,1);
    list<wstring> sens = semdb.get_word_senses(form,lem,pos);
    bool found=false;
    for (list<wstring>::iterator s=sens.begin(); !found && s!=sens.end(); s++) {
      found=find_any(semdb.get_sense_info(*s).tonto);
    }
    return(found);
  }

  /// check semantic file

  check_semfile::check_semfile(semanticDB &db, const wstring &n, const wstring &f) : rule_expression(n,f), semdb(db) {}
  bool check_semfile::eval (dep_tree::iterator n) const {
    wstring form=n->info.get_word().get_lc_form();
    wstring lem=n->info.get_word().get_lemma();
    wstring pos=n->info.get_word().get_tag().substr(0,1);
    list<wstring> sens = semdb.get_word_senses(form,lem,pos);
    bool found=false;
    for (list<wstring>::iterator s=sens.begin(); !found && s!=sens.end(); s++) {
      found=find(semdb.get_sense_info(*s).semfile);
    }
    return(found);
  }

  /// check synonyms

  check_synon::check_synon(semanticDB &db, const wstring &n, const wstring &w) : rule_expression(n,w), semdb(db) {}
  bool check_synon::eval (dep_tree::iterator n) const {
    wstring form=n->info.get_word().get_lc_form();
    wstring lem=n->info.get_word().get_lemma();
    wstring pos=n->info.get_word().get_tag().substr(0,1);
    list<wstring> sens = semdb.get_word_senses(form,lem,pos);
    bool found=false;
    for (list<wstring>::iterator s=sens.begin(); !found && s!=sens.end(); s++) {
      found=find_any(semdb.get_sense_info(*s).words);
    }
    return(found);
  }

  /// check ancestor synonyms

  check_asynon::check_asynon(semanticDB &db, const wstring &n, const wstring &w) : rule_expression(n,w), semdb(db) {}
  bool check_asynon::eval (dep_tree::iterator n) const {
    wstring form=n->info.get_word().get_lc_form();
    wstring lem=n->info.get_word().get_lemma();
    wstring pos=n->info.get_word().get_tag().substr(0,1);

    // start checking for a synonym in the senses of the word
    list<wstring> sens = semdb.get_word_senses(form,lem,pos);
    bool found=false;
    for (list<wstring>::iterator s=sens.begin(); !found && s!=sens.end(); s++) {
      found=find_any(semdb.get_sense_info(*s).words);
      // if not found, enlarge the list of senses to explore
      // with all parents of the current sense
      if (!found) {
        list<wstring> lpar=semdb.get_sense_info(*s).parents;
        if (lpar.size()>0) sens.splice(sens.end(),lpar);
      }
    }
    return(found);
  }


  //---------- Class ruleLabeler -------------------

  ////////////////////////////////////////////////////////////////
  ///  Constructor
  ////////////////////////////////////////////////////////////////

  ruleLabeler::ruleLabeler(void) {};

  ////////////////////////////////////////////////////////////////
  ///  Constructor
  ////////////////////////////////////////////////////////////////

  ruleLabeler::ruleLabeler(const wstring &plabel, rule_expression* pre) {
    re=pre;
    label=plabel; 
  }

  ////////////////////////////////////////////////////////////////
  ///  Evaluate rule conditions
  ////////////////////////////////////////////////////////////////

  bool ruleLabeler::check(dep_tree::iterator ancestor, dep_tree::iterator descendant) const {
    TRACE(4,L"      eval ruleLabeler");
    return re->check(ancestor, descendant);
  }

} // namespace

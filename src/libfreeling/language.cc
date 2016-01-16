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

#include "freeling/morfo/language.h"
#include "freeling/morfo/util.h"

using namespace std;

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///   Class analysis stores a possible reading (lemma, PoS, probability, distance)
  ///   for a word.
  ////////////////////////////////////////////////////////////////

  /// Create empty analysis
  analysis::analysis() {};
  /// Create a new analysis with provided data.
  analysis::analysis(const wstring &l, const wstring &p) {lemma=l; tag=p; prob= -1.0; distance= -1.0;}
  /// Assignment
  analysis& analysis::operator=(const analysis &a) {
    if(this!=&a) {
      lemma=a.lemma; tag=a.tag; prob=a.prob; distance=a.distance;
      senses=a.senses; retok=a.retok; user=a.user;
      selected_kbest=a.selected_kbest;
    }
    return *this;
  }

  // Set lemma, tag and default properties
  void analysis::init(const std::wstring &l, const std::wstring &t)
  {
    this->lemma=l; this->tag=t; this->prob=-1.0; this->distance= -1.0;
    user.clear();
    selected_kbest.clear();
    retok.clear();
    senses.clear();
  }
  /// Set lemma for analysis.
  void analysis::set_lemma(const wstring &l) {lemma=l;}
  /// Set PoS tag for analysis
  void analysis::set_tag(const wstring &p) {tag=p;}
  /// Set probability for analysis
  void analysis::set_prob(double p) { prob=p;}
  /// Set distance for analysis
  void analysis::set_distance(double d) { distance=d;}
  /// Set retokenization info for analysis
  void analysis::set_retokenizable(const list<word> &lw) {retok=lw;}
  /// Check whether probability has been set.
  bool analysis::has_prob() const {return(prob>=0.0);}
  /// Check whether distance has been set.
  bool analysis::has_distance() const {return(distance>=0.0);}
  /// Get lemma value for analysis.
  wstring analysis::get_lemma() const {return(lemma);}
  /// Get probability value for analysis (-1 if not set).
  double analysis::get_prob() const {return(prob);}
  /// Get distance value for analysis (-1 if not set).
  double analysis::get_distance() const {return(distance);}
  /// Find out if the analysis may imply retokenization
  bool analysis::is_retokenizable() const {return(!retok.empty());}
  /// Get retokenization info for analysis.
  list<word>& analysis::get_retokenizable() {return(retok);}
  /// Get retokenization info for analysis.
  const list<word>& analysis::get_retokenizable() const {return(retok);}
  /// Get PoS tag value for analysis.
  wstring analysis::get_tag() const {return(tag);}
  /// get analysis sense list
  const list<pair<wstring, double> >& analysis::get_senses() const {return(senses);};
  /// get ref to analysis sense list
  list<pair<wstring, double> >& analysis::get_senses() {return(senses);};
  wstring analysis::get_senses_string() const {return(util::pairlist2wstring(senses,L":",L"/"));};
  /// set analiysis sense list
  void analysis::set_senses(const list<pair<wstring, double> > &ls) {senses=ls;};
  // get the largest kbest sequence index the analysis is selected in.
  int analysis::max_kbest() const { 
    if (selected_kbest.empty()) return 0; 
    else return *(--selected_kbest.end()); 
  }
  /// find out whether the analysis is selected in the tagger k-th best sequence
  bool analysis::is_selected(int k) const {return selected_kbest.find(k)!=selected_kbest.end();}
  /// mark this analysis as selected in k-th best sequence
  void analysis::mark_selected(int k) {selected_kbest.insert(k);}
  /// unmark this analysis as selected in k-th best sequence
  void analysis::unmark_selected(int k) {selected_kbest.erase(k);}

  /// Comparison to sort analysis by *ascending* probability and ascending alphabetical tag
  bool analysis::operator>(const analysis &a) const {return(prob>a.prob or (prob==a.prob and tag<a.tag));}
  /// Comparison to sort analysis by *descending* probability and ascending alphabetical tag
  bool analysis::operator<(const analysis &a) const {return(prob<a.prob or (prob==a.prob and tag<a.tag));}
  /// comparison (just to please MSVC)
  bool analysis::operator==(const analysis &a) const {return(lemma==a.lemma && tag==a.tag);}


  ////////////////////////////////////////////////////////////////
  ///   Class word stores all info related to a word:
  ///  form, list of analysis, list of tokens (if multiword).
  ////////////////////////////////////////////////////////////////

  /// Create an empty new word
  word::word() { }

  /// Create a new word with given form.
  word::word(const wstring &f) {
    form=f;
    ph_form=L"";
    lc_form=util::lowercase(f);
    in_dict=true;  // everything is a known word until dictionary fails to find it.
    locked=false; 
    ambiguous_mw=false;
    position=-1;
  }
  /// Create a new multiword, including given words.
  word::word(const wstring &f, const list<word> &a) {
    form=f; 
    ph_form=L"";
    lc_form=util::lowercase(f);
    multiword=a;
    start=a.front().get_span_start();
    finish=a.back().get_span_finish();
    in_dict=true;
    locked=false;
    ambiguous_mw=false;
    position=-1;
  }
  /// Create a new multiword, including given words, and with given analysis list.
  word::word(const wstring &f, const list<analysis> &la, const list<word> &a) {
    word::const_iterator i;
    this->clear(); 
    for (i=la.begin(); i!=la.end(); i++) {
      this->push_back(*i);
      this->back().mark_selected();
    }
    form=f; 
    lc_form=util::lowercase(f);
    ph_form=L"";
    multiword=a;
    start=a.front().get_span_start();
    finish=a.back().get_span_finish();
    in_dict=true;
    locked=false;
    ambiguous_mw=false;
    position=-1;
  }

  /// Clone word
  void word::clone(const word &w) {
    form=w.form; lc_form=w.lc_form;
    ph_form=w.ph_form;
    multiword=w.multiword;
    start=w.start; finish=w.finish;
    in_dict=w.in_dict;
    locked=w.locked;
    user=w.user;
    alternatives=w.alternatives;
    copy_analysis(w);
    ambiguous_mw=w.ambiguous_mw;
    position=w.position;
  }

  /// Copy constructor.
  word::word(const word & w) { clone(w);};

  /// Assignment.
  word& word::operator=(const word& w){
    if (this!=&w) clone(w);
    return *this;
  };

  /// Copy analysis list of given word.
  void word::copy_analysis(const word &w) {
    word::const_iterator i;
    // copy all analysis
    this->clear();
    for (i=w.begin(); i!=w.end(); i++) 
      this->push_back(*i);
  }
  /// Set (override) word analysis list with one single analysis
  void word::set_analysis(const analysis &a) {
    this->clear(); 
    this->push_back(a); 
    this->back().mark_selected();
  }

  /// Set (override) word analysis list.
  void word::set_analysis(const list<analysis> &a) {
    list<analysis>::const_iterator i;
    this->clear();
    for (i=a.begin(); i!=a.end(); ++i) {
      this->push_back(*i);
      this->back().mark_selected();
    }
  } 

  /// Add one analysis to word analysis list.
  void word::add_analysis(const analysis &a) {
    this->push_back(a);
    this->back().mark_selected();
  }
  /// Set word form.
  void word::set_form(const wstring &f) {form=f;  lc_form=util::lowercase(f);}
  void word::set_ph_form(const wstring &f) {ph_form=f;}
  /// Set token span.
  void word::set_span(unsigned long s, unsigned long e) {start=s; finish=e;}

  /// get in_dict
  bool word::found_in_dict() const {return (in_dict);}
  /// set in_dict
  void word::set_found_in_dict(bool b) {in_dict=b;}
  /// check if there is any retokenizable analysis
  bool word::has_retokenizable() const {
    word::const_iterator i;
    bool has=false;
    for (i=this->begin(); i!=this->end() && !has; i++) has=i->is_retokenizable();
    return(has);
  }

  /// add an alternative to the alternatives list
  void word::add_alternative(const wstring &w, int d) { alternatives.push_back(make_pair(w,d)); }
  /// replace alternatives list with list given
  void word::set_alternatives(const list<pair<wstring,int> > &lw) { alternatives=lw; }
  /// clear alternatives list
  void word::clear_alternatives() { alternatives.clear(); }
  /// find out if the speller checked alternatives
  bool word::has_alternatives() const {return (alternatives.size()>0);}
  /// get alternatives list const &
  const list<pair<wstring,int> >& word::get_alternatives() const {return(alternatives);}
  /// get alternatives list &
  list<pair<wstring,int> >& word::get_alternatives() {return(alternatives);}
  /// get alternatives begin iterator
  list<pair<wstring,int> >::iterator word::alternatives_begin() {return alternatives.begin();}
  /// get alternatives end iterator
  list<pair<wstring,int> >::iterator word::alternatives_end() {return alternatives.end();}
  /// get alternatives begin const iterator
  list<pair<wstring,int> >::const_iterator word::alternatives_begin() const {return alternatives.begin();}
  /// get alternatives end const iterator
  list<pair<wstring,int> >::const_iterator word::alternatives_end() const {return alternatives.end();}

  /// mark word as having definitive analysis
  void word::lock_analysis() { locked=true; }
  /// check if word is marked as having definitive analysis
  bool word::is_locked() const { return locked; }
  /// Get length of analysis list.
  int word::get_n_analysis() const {return(this->size());}
  /// get list of analysis (only useful for perl API)
  list<analysis> word::get_analysis() const {return(*this);};
  /// get begin iterator to analysis list.
  word::iterator word::analysis_begin() {return this->begin();}
  word::const_iterator word::analysis_begin() const {return this->begin();}
  /// get end iterator to analysis list.
  word::iterator word::analysis_end() {return this->end();}
  word::const_iterator word::analysis_end() const {return this->end();}
  /// mark all analysis as selected for k-th best sequence
  void word::select_all_analysis(int k) {
    for (word::iterator i=this->begin(); i!=this->end(); i++) i->mark_selected(k);
  }
  /// un mark all analysis as selected for k-th best sequence
  void word::unselect_all_analysis(int k) {
    for (word::iterator i=this->begin(); i!=this->end(); i++) i->unmark_selected(k);
  }
  /// Mark given analysis as selected.
  void word::select_analysis(word::iterator tag, int k) { tag->mark_selected(k); }
  /// Unmark given analysis as selected.
  void word::unselect_analysis(word::iterator tag, int k) { tag->unmark_selected(k); }
  /// Get the number of selected analysis
  int word::get_n_selected(int k) const {
    int n=0;
    for (list<analysis>::const_iterator i=this->begin(); i!=this->end(); i++)
      if (i->is_selected(k)) n++;
    return(n);
  }
  /// Get the number of unselected analysis
  int word::get_n_unselected(int k) const {return(this->size() - this->get_n_selected(k));}
  /// Check whether the word is a compound.
  bool word::is_multiword() const {return(!multiword.empty());}
  /// Check whether the word is an ambiguous multiword
  bool word::is_ambiguous_mw() const {return(ambiguous_mw);}
  /// Set mw ambiguity status
  void word::set_ambiguous_mw(bool a) {ambiguous_mw=a;}
  /// Get number of words in compound.
  int word::get_n_words_mw() const {return(multiword.size());}
  /// Get list of words in compound.
  const list<word>& word::get_words_mw() const {return(multiword);}
  /// Get word form.
  wstring word::get_form() const {return(form);}
  /// Get word form, lowercased.
  wstring word::get_lc_form() const {return(lc_form);}
  ///Get word phonetic form.
  wstring word::get_ph_form() const {return(ph_form);}
  /// Get the first selected analysis iterator
  word::iterator word::selected_begin(int k) {
    list<analysis>::iterator p;
    p=this->begin();
    while (p!=this->end() and not p->is_selected(k)) p++;
    return word::iterator(this->begin(),this->end(),p,SELECTED,k);
  }
  /// Get the first selected analysis iterator
  word::const_iterator word::selected_begin(int k) const {
    list<analysis>::const_iterator p;
    p=this->begin();
    while (p!=this->end() and not p->is_selected(k)) p++;
    return word::const_iterator(this->begin(),this->end(),p,SELECTED,k);
  }
  /// Get the end of selected analysis list
  word::iterator word::selected_end(int k) {return(this->end());}
  /// Get the end of selected analysis list
  word::const_iterator word::selected_end(int k) const {return(this->end());}
  /// Get the first unselected analysis iterator
  word::iterator word::unselected_begin(int k) {
    list<analysis>::iterator p;
    p=this->begin();
    while (p!=this->end() and p->is_selected(k)) p++;
    return word::iterator(this->begin(),this->end(),p,UNSELECTED,k);
  }
  /// Get the first unselected analysis iterator
  word::const_iterator word::unselected_begin(int k) const {
    list<analysis>::const_iterator p;
    p=this->begin();
    while (p!=this->end() and p->is_selected(k)) p++;
    return word::const_iterator(this->begin(),this->end(),p,UNSELECTED,k);
  }
  /// Get the end of unselected analysis list
  word::iterator word::unselected_end(int k) {return(this->end());}
  /// Get the end of unselected analysis list
  word::const_iterator word::unselected_end(int k) const {return(this->end());}
  /// Get how many kbest tags the word stores
  unsigned int word::num_kbest() const {
    unsigned int mx= 0;
    for (list<analysis>::const_iterator a=this->begin(); a!=this->end(); a++) {
      unsigned int y = (unsigned int)(a->max_kbest()+1);
      mx = (y>mx? y : mx);
    }
    return mx;
  }
  /// Get lemma for the selected analysis in list.
  wstring word::get_lemma(int k) const {
    return (this->get_n_analysis() ? selected_begin(k)->get_lemma() : L"");
  }
  /// Get PoS tag for the selected analysis in list.
  wstring word::get_tag(int k) const {
    return (this->get_n_analysis() ? selected_begin(k)->get_tag() : L"");
  }
  /// get reference to sense list for the selected analysis
  const list<pair<wstring,double> > & word::get_senses(int k) const {
    return(selected_begin(k)->get_senses());
  };
  /// get reference to sense list for the selected analysis
  list<pair<wstring,double> > & word::get_senses(int k) {
    return(selected_begin(k)->get_senses());
  };
  /// get sense list (as string) for the selected analysis
  wstring word::get_senses_string(int k) const {
    return(util::pairlist2wstring(selected_begin(k)->get_senses(),L":", L"/"));
  };
  /// set sense list for the selected analysis
  void word::set_senses(const list<pair<wstring,double> > &ls, int k) {
    selected_begin(k)->set_senses(ls);
  };

  // get/set position of word in sentence (start from 0)
  void word::set_position(size_t p) { position=p; }
  size_t word::get_position() const { return position; }

  /// Get token span.
  unsigned long word::get_span_start() const {return(start);}
  unsigned long word::get_span_finish() const {return(finish);}
  /// look for a tag in the analysis list of a word
  bool word::find_tag_match(const freeling::regexp &re) const {
    bool found=false;
    for (word::const_iterator an=this->begin(); an!=this->end() && !found; an++) 
      found = re.search(an->get_tag());
    return found;
  }


  //////////////////////////////////////////////////////////////////
  ///   word::iterator may act as basic list iterator over all   ///
  ///   analysis of a word, or may act as a filtered iterator,   ///
  ///   providing access only to selected/unselected analysis    ///
  //////////////////////////////////////////////////////////////////

  /// Empty constructor
  word::iterator::iterator() : type(ALL),kbest(0) {}
  /// Copy
  word::iterator::iterator(const word::iterator &x) : list<analysis>::iterator(x) {
    type=x.type; kbest=x.kbest; 
    ibeg=x.ibeg; iend=x.iend;
  }
  /// Constructor from std::list iterator
  word::iterator::iterator(const list<analysis>::iterator &x) 
    : list<analysis>::iterator(x),type(ALL),kbest(0) {}
  /// Constructor for filtered iterators (selected/unselected)
  word::iterator::iterator(const list<analysis>::iterator &b, 
                           const list<analysis>::iterator &e, 
                           const list<analysis>::iterator &x,
                           int t, int k) : list<analysis>::iterator(x),ibeg(b),iend(e),type(t),kbest(k) {}
  /// Generic preincrement, for all cases
  word::iterator& word::iterator::operator++() {
    do {
      this->list<analysis>::iterator::operator++();
    } while (type!=ALL and (*this)!=iend and (*this)->is_selected(kbest)!=(type==SELECTED) );
    return (*this);
  }
  /// Generic postincrement, for all cases
  word::iterator word::iterator::operator++(int) {
    word::iterator b=(*this);
    ++(*this);
    return b;
  }

  //////////////////////////////////////////////////////////////////////////
  ///  word::const_iterator is the same than word::iterator,  but const  ///
  //////////////////////////////////////////////////////////////////////////

  /// Empty constructor
  word::const_iterator::const_iterator() : type(ALL),kbest(0) {}
  /// Copy
  word::const_iterator::const_iterator(const word::const_iterator &x) : list<analysis>::const_iterator(x) {
    type=x.type; kbest=x.kbest; 
    ibeg=x.ibeg; iend=x.iend;
  }
  /// Copy from nonconst iterator
  word::const_iterator::const_iterator(const word::iterator &x) : list<analysis>::const_iterator(x) {
    type=x.type; kbest=x.kbest; 
    ibeg=x.ibeg; iend=x.iend;  
  }
  /// Constructor from std::list iterator
  word::const_iterator::const_iterator(const list<analysis>::const_iterator &x) 
    : list<analysis>::const_iterator(x),type(ALL),kbest(0) {}
  /// Constructor from nonconst std::list iterator
  word::const_iterator::const_iterator(const list<analysis>::iterator &x) 
    : list<analysis>::const_iterator(x),type(ALL),kbest(0) {}
  /// Constructor for filtered iterators (selected/unselected)
  word::const_iterator::const_iterator(const list<analysis>::const_iterator &b, 
                                       const list<analysis>::const_iterator &e, 
                                       const list<analysis>::const_iterator &x,
                                       int t,int k) : list<analysis>::const_iterator(x),ibeg(b),iend(e),type(t),kbest(k) {}
  /// Generic preincrement, for all cases
  word::const_iterator& word::const_iterator::operator++() {
    do {
      this->list<analysis>::const_iterator::operator++();
    } while (type!=ALL and (*this)!=iend and (*this)->is_selected(kbest)!=(type==SELECTED) );  
    return (*this);
  }
  /// Generic increment, for all cases
  word::const_iterator word::const_iterator::operator++(int) {
    word::const_iterator b=(*this);
    ++(*this);
    return b;
  }

  ////////////////////////////////////////////////////////////////
  ///   Class node stores nodes of a parse_tree
  ///  Each node in the tree is either a label (intermediate node)
  ///  or a word (leaf node)
  ////////////////////////////////////////////////////////////////

  /// Methods for parse_tree nodes
  node::node(const wstring & s) : label(s), w(NULL) {head=false;chunk=false;nodeid=L"-";}
  node::node() : label(L""), w(NULL) {head=false;chunk=false;nodeid=L"-";}
  wstring node::get_node_id() const {return (nodeid);}
  void node::set_node_id(const wstring &id) {nodeid=id;}
  wstring node::get_label() const {return(label);}
  const word& node::get_word() const {return (*w);}
  word& node::get_word() {return (*w);}
  void node::set_label(const wstring &s) {label=s;}
  void node::set_word(word &wd)  {w = &wd;}
  bool node::is_head() const {return head;}
  void node::set_head(const bool h) {head=h;}
  bool node::is_chunk() const {return (chunk!=0);}
  void node::set_chunk(const int c) {chunk=c;}
  int  node::get_chunk_ord() const {return (chunk);}


  ////////////////////////////////////////////////////////////////
  ///   Class parse tree is used to store the results of parsing
  ////////////////////////////////////////////////////////////////

  /// Methods for parse_tree
  parse_tree::parse_tree() : tree<node>() {}
  parse_tree::parse_tree(parse_tree::iterator p) : tree<node>(p) {}
  parse_tree::parse_tree(const node & n) : tree<node>(n) {}
  /// assign id's to nodes and build index
  void parse_tree::build_node_index(const wstring &sid) {
    parse_tree::iterator k;
    int i=0;
    node_index.clear();
    for (k=this->begin(); k!=this->end(); ++k, i++) {
      wstring id=sid+L"."+util::int2wstring(i);
      k->info.set_node_id(id);
      node_index.insert(make_pair(id,k));
    }
  }
  // rebuild index maintaining id's
  void parse_tree::rebuild_node_index() {
    node_index.clear();
    word_index.clear();
    for (parse_tree::iterator k=this->begin(); k!=this->end(); ++k) {
      wstring id=k->info.get_node_id();
      if (id != L"-") node_index.insert(make_pair(id,k));
      if (k->num_children()==0) word_index.push_back(k);
    }
  }

  // auxiliary to factorize const/noconst "get_node_by_id"
  #define pt_node_by_id(ITER_TYPE)  {map<wstring,parse_tree::iterator>::ITER_TYPE p = node_index.find(id); \
                                  return (p!=node_index.end() ? p->second : (tree<node>*)NULL);}
  /// get node with given index, normal iterator
  parse_tree::iterator parse_tree::get_node_by_id(const wstring & id) { pt_node_by_id(iterator); }
  /// get node with given index, const iterator
  parse_tree::const_iterator parse_tree::get_node_by_id(const wstring & id) const { pt_node_by_id(const_iterator); }

  // auxiliary to factorize const/noconst "get_node_by_pos"
  #define pt_node_by_pos(pos)  {return word_index[pos];}
  /// get leaf node corresponding to word at given position in sentence, normal iterator
  parse_tree::iterator parse_tree::get_node_by_pos(size_t pos) { pt_node_by_pos(pos); }
  /// get leaf node corresponding to word at given position in sentence, const iterator
  parse_tree::const_iterator parse_tree::get_node_by_pos(size_t pos) const { pt_node_by_pos(pos); }


  ////////////////////////////////////////////////////////////////
  /// class denode stores nodes of a dependency tree and
  ///  parse tree <-> deptree relations
  ////////////////////////////////////////////////////////////////

  /// Methods for dependency tree nodes
  depnode::depnode() {};
  depnode::depnode(const wstring & s) : node(s),link(NULL) {}
  depnode::depnode(const node & n) : node(n),link(NULL) {}
  void depnode::set_link(const parse_tree::iterator p) {link=p;}
  parse_tree::iterator depnode::get_link() { return link;}
  parse_tree::const_iterator depnode::get_link() const { return link;}
  tree<node>& depnode::get_link_ref() { return (*link);}  ///  (useful for Java API)

  ////////////////////////////////////////////////////////////////
  /// class dep_tree stores a dependency tree
  ////////////////////////////////////////////////////////////////

  /// Constructors for dep_tree
  dep_tree::dep_tree() : tree<depnode>() {};
  dep_tree::dep_tree(const depnode & n) : tree<depnode>(n) {};

  // auxiliary to factorize const/noconst "get_node_by_pos"
  #define dt_node_by_pos(pos)  {return word_index[pos];}
  /// get depnode corresponding to word in given position, const iterator
  dep_tree::const_iterator dep_tree::get_node_by_pos(size_t pos) const { dt_node_by_pos(pos); }
  /// get depnode corresponding to word in given position, normal iterator
  dep_tree::iterator dep_tree::get_node_by_pos(size_t pos) { dt_node_by_pos(pos); }

  /// rebuild index maintaining word positions
  void dep_tree::rebuild_node_index() {
    word_index.clear();
    for (dep_tree::iterator d=this->begin(); d!=this->end(); ++d) {
      size_t pos=d->info.get_word().get_position();
      if (pos>=word_index.size()) word_index.resize(pos+1, dep_tree::iterator());
      word_index[pos]=d;
    }  
  }


  ////////////////////////////////////////////////////////////////
  /// Virtual class to store the processing state of a sentence.
  /// Each processor will define a derived class with needed contents,
  /// and store it in the sentence being processed.
  ////////////////////////////////////////////////////////////////

  processor_status::processor_status() {};

  ////////////////////////////////////////////////////////////////
  ///   Class sentence is just a list of words that someone
  /// (the splitter) has validated it as a complete sentence.
  /// It may include a parse tree.
  ////////////////////////////////////////////////////////////////

  /// Create a new sentence.
  sentence::sentence() { pts.clear(); dts.clear(); wpos.clear(); status.clear(); sent_id=L"0";};
  /// Create a new sentence from list<word>
  sentence::sentence(const list<word> &lw) : list<word>(lw) { 
    pts.clear(); dts.clear(); status.clear(); sent_id=L"0";
    rebuild_word_index();
  }
  /// add a word to the sentence
  void sentence::push_back(const word &w) {
    this->list<word>::push_back(w);
    this->back().set_position(this->size()-1);
    wpos.push_back(&(this->back()));
  }

  /// rebuild word index by position
  void sentence::rebuild_word_index() {
    wpos = vector<word*>(this->size(),(word*)NULL);
    size_t i=0;
    for (sentence::iterator w=this->begin(); w!=this->end(); w++) {
      wpos[i]= &(*w);
      w->set_position(i);
      i++;
    }

    // if there is a constituency tree, index its leaf nodes by position
    if (this->is_parsed()) {
      for (map<int,parse_tree>::iterator k=pts.begin(); k!=pts.end(); k++) 
        k->second.rebuild_node_index();
    }

    // if there is a dependency tree, index its nodes by position
    if (this->is_dep_parsed()) {
      for (map<int,dep_tree>::iterator k=dts.begin(); k!=dts.end(); k++) 
        k->second.rebuild_node_index();
    } 
  }

  /// Clone sentence
  void sentence::clone(const sentence &s) {
    // copy processing status
    status = s.status;
    // copy identifier
    sent_id = s.sent_id;
    // copy word list
    wpos = vector<word*>(s.size(),(word*)NULL);
    map<const word*,word*> wps;
    this->list<word>::clear(); 
    int i=0;
    for (sentence::const_iterator w=s.begin(); w!=s.end(); w++) {
      this->push_back(*w);
      // remember pointers to new and old words, to update trees (see below).
      wps.insert(make_pair(&(*w),&(this->back())));
      // store word pointer for positional acces
      wpos[i++] = &(this->back());
    }

    // copy parse trees, and fix pointers to words in leaves
    pts=s.pts; 
    for (map<int,parse_tree>::iterator k=pts.begin(); k!=pts.end(); k++) {
      sentence::iterator j=this->begin();
      for (parse_tree::iterator p=k->second.begin(); p!=k->second.end(); ++p) {
        if (p->num_children()==0) {
          p->info.set_word(*j);
          j++;
        }
      }
      k->second.rebuild_node_index();
    }

    // copy dependency trees, and fix links to parse_tree and words.
    dts=s.dts;
    for (map<int,dep_tree>::iterator k=dts.begin(); k!=dts.end(); k++) {
      for (dep_tree::iterator d=k->second.begin(); d!=k->second.end(); ++d) {
        // update link to parse_tree, if any
        if (d->info.get_link()!=NULL) {
          wstring id=d->info.get_link()->info.get_node_id();
          parse_tree::iterator p=pts[k->first].get_node_by_id(id);
          d->info.set_link(p);
        }
        // update link to word
        word &w=d->info.get_word();   
        d->info.set_word(*wps[&w]);
      }
      k->second.rebuild_node_index();
    }

  }

  /// Copy constructor.
  sentence::sentence(const sentence & s) { clone(s); };
  /// Assignment.
  sentence& sentence::operator=(const sentence& s) {
    if (this!=&s) clone(s);
    return *this;
  }
  /// positional access to a word
  const word& sentence::operator[](size_t i) const { return (*wpos[i]); }
  word& sentence::operator[](size_t i) { return (*wpos[i]); }
  /// find out how many kbest sequences the tagger computed
  unsigned int sentence::num_kbest() const {
    if (this->empty()) return 0;
    else return this->begin()->num_kbest();
  }
  /// Clear sentence and possible trees
  void sentence::clear() { 
    this->list<word>::clear(); 
    pts.clear(); dts.clear();
    wpos.clear(); 
    while (not status.empty()) clear_processing_status();
  }

  /// Set sentence identifier
  void sentence::set_sentence_id(const wstring &sid) {sent_id=sid;}
  /// Get sentence identifier
  wstring sentence::get_sentence_id() {return sent_id;}

  /// Set the parse tree.
  void sentence::set_parse_tree(const parse_tree &tr, int k) {
    pts[k]=tr;
    pts[k].rebuild_node_index();
  };
  /// Obtain the parse tree.
  parse_tree & sentence::get_parse_tree(int k) {return pts[k];};
  const parse_tree & sentence::get_parse_tree(int k) const {
    map<int,parse_tree>::const_iterator t=pts.find(k);
    return t->second;
  }
  /// Find out whether the sentence is parsed.
  bool sentence::is_parsed() const {return not pts.empty();};
  /// Set the dependency tree.
  void sentence::set_dep_tree(const dep_tree &tr, int k) {
    dts[k]=tr;
    dts[k].rebuild_node_index();
  };
  /// Obtain the parse dependency tree
  dep_tree & sentence::get_dep_tree(int k) {return dts[k];};
  const dep_tree & sentence::get_dep_tree(int k) const {
    map<int,dep_tree>::const_iterator t=dts.find(k);
    return t->second;
  }
  /// Find out whether the sentence is dependency parsed.
  bool sentence::is_dep_parsed() const {return not dts.empty();};
  /// obtain list of words (useful for perl APIs)
  vector<word> sentence::get_words() const {
    vector<word> v;
    for (sentence::const_iterator i=this->begin(); i!=this->end(); i++)
      v.push_back(*i);
    return (v);
  }

  /// get/set processing status
  processor_status* sentence::get_processing_status() {return status.back();}
  const processor_status* sentence::get_processing_status() const {return status.back();}
  void sentence::set_processing_status(processor_status *s) {status.push_back(s);}
  void sentence::clear_processing_status() {
    if (status.empty()) return; 
    processor_status *s=status.back(); 
    status.pop_back(); 
    delete s;
  }

  /// obtain iterators (useful for perl/java APIs)
  sentence::iterator sentence::words_begin() {return this->begin();}
  sentence::const_iterator sentence::words_begin() const {return this->begin();}
  sentence::iterator sentence::words_end() {return this->end();}
  sentence::const_iterator sentence::words_end() const {return this->end();}


  ////////////////////////////////////////////////////////////////
  ///   Class paragraph is just a list of sentences that someone
  ///  has validated it as a paragraph.
  ////////////////////////////////////////////////////////////////


  ////////////////////////////////////////////////////////////////
  ///   Class document is a list of paragraphs. It may have additional
  ///  information (such as title)
  ////////////////////////////////////////////////////////////////

  /// Constructor
  document::document() {}

  /// Add node1 to the group group1 of coreferents
  void document::add_positive(const wstring &node1, int group1) {
    group2node.insert( make_pair(group1,node1) );
    node2group[node1] = group1;
  }

  /// Add second node to coreference group of the first (or in a new group if first didn't have any)
  void document::add_positive(const wstring &node1, const wstring &node2) {
    // find out group of node1, creating a new group if it didn't have any.
    int g1 = get_coref_group(node1);
    if (g1 == -1) {
      g1 = ++last_group;
      group2node.insert( make_pair(g1,node1) );
      node2group[node1] = g1;
    }
    // add second node to group g1
    group2node.insert( make_pair(g1,node2) );
    node2group[node2] = g1;
  }

  /// Gets the id of the coreference group of the node
  int document::get_coref_group(const wstring &node1) const {
    map<wstring,int>::const_iterator it = node2group.find(node1);
    if (it == node2group.end()) return -1;
    else return it->second;
  }

  /// Gets all the nodes in a coreference group id
  list<wstring> document::get_coref_nodes(int id) const {
    list<wstring> ret;
    multimap<int,wstring>::const_iterator it;
    pair<multimap<int,wstring>::const_iterator,multimap<int,wstring>::const_iterator> par;
    par = group2node.equal_range(id);
    for (it=par.first; it!=par.second; ++it) ret.push_back(it->second);
    return(ret);
  }

  /// Returns whether two nodes are in the same coreference group
  bool document::is_coref(const wstring &node1, const wstring &node2) const {
    int g1, g2;
    g1 = get_coref_group(node1);
    g2 = get_coref_group(node2);
    return ( g1!= -1 && g1==g2 );
  }
} // namespace

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


#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/dep_rules.h"
#include "freeling/morfo/dependencies.h"
#include "freeling/morfo/configfile.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"DEP_TXALA"
#define MOD_TRACECODE DEP_TRACE


  //---------- Class completer ----------------------------------

  ///////////////////////////////////////////////////////////////
  /// Constructor. Load a tree-completion grammar 
  ///////////////////////////////////////////////////////////////

  completer::completer(const wstring &filename) {

    freeling::regexp blankline(L"^[ \t]*$");
    wstring path=filename.substr(0,filename.find_last_of(L"/\\")+1);
  
    enum sections {CLASS,GRPAR};
    config_file cfg(true,L"%");  
    cfg.add_section(L"CLASS",CLASS);
    cfg.add_section(L"GRPAR",GRPAR);

    if (not cfg.open(filename))
      ERROR_CRASH(L"Error opening completer file "+filename);

    wstring line;
    wordclasses.clear();

    while (cfg.get_content_line(line)) {

      int lnum = cfg.get_line_num();

      switch (cfg.get_section()) {

      case CLASS: {
        // load CLASS section
        wstring vclass, vlemma;
        wistringstream sin;  sin.str(line);
        sin>>vclass>>vlemma;
        completer::load_classes(vclass, vlemma, path, wordclasses);
        break;
      }

      case GRPAR: {
        ////// load GRPAR section defining tree-completion rules
        completerRule r;
        r.line=lnum;
        
        wistringstream sline; sline.str(line);
        wstring flags,chunks,lit,newlabels,context;
        sline>>r.weight>>flags>>context>>chunks>>r.operation>>lit>>newlabels;
        r.enabling_flags=util::wstring2set(flags,L"|");
        
        if (lit==L"RELABEL") {
          if (newlabels==L"-") {
            r.newNode1 = L"-"; 
            r.newNode2 = L"-";
          }
          else {
            wstring::size_type p=newlabels.find(L":");
            if (p!=wstring::npos) {
              r.newNode1 = newlabels.substr(0,p);
              r.newNode2 = newlabels.substr(p+1);       
            }
            else {
              WARNING(L"Invalid RELABEL value in completer rule at line "+util::int2wstring(lnum)+L". Rule will be ignored");
              continue;
            }
          }
        }
        else if (lit==L"MATCHING") {
          extract_conds(newlabels,r.matchingCond);
        }
        else {
          WARNING(L"Invalid operation '"+lit+L"' in completer rule at line "+util::int2wstring(lnum)+L". Rule will be ignored");
          continue;
        }
        
        // read flags to activate/deactivate until line ends or a comment is found
        bool comm=false;
        while (!comm && sline>>flags) {
          if (flags[0]==L'%') comm=true;
          else if (flags[0]==L'+') r.flags_toggle_on.insert(flags.substr(1));
          else if (flags[0]==L'-') r.flags_toggle_off.insert(flags.substr(1));
          else WARNING(L"Syntax error reading completer rule at line "+util::int2wstring(lnum)+L". Flag must be toggled either on (+) or off (-)");
        }
        
        if ((r.operation==L"top_left" || r.operation==L"top_right") && lit!=L"RELABEL")
          WARNING(L"Syntax error reading completer rule at line "+util::int2wstring(lnum)+L". "+r.operation+L" requires RELABEL.");
        if ((r.operation==L"last_left" || r.operation==L"last_right" || r.operation==L"cover_last_left") && lit!=L"MATCHING")     
          WARNING(L"Syntax error reading completer rule at line "+util::int2wstring(lnum)+L". "+r.operation+L" requires MATCHING.");
        
        if (context==L"-") context=L"$$";
        r.context_neg=false;
        if (context[0]==L'!') {
          r.context_neg=true;
          context=context.substr(1);
        }
        
        vector<wstring> conds = util::wstring2vector(context,L"_");
        bool left=true;
        for (size_t c=0; c<conds.size(); c++) {
          if (conds[c]==L"$$") {left=false; continue;}
          
          matching_condition mc;
          extract_conds(conds[c],mc);
          if (left) r.leftContext.push_back(mc);
          else r.rightContext.push_back(mc);
        }
        
        wstring::size_type p=chunks.find(L",");
        if (chunks[0]!=L'(' || chunks[chunks.size()-1]!=L')' || p==wstring::npos)  
          WARNING(L"Syntax error reading completer rule at line "+util::int2wstring(lnum)+L". Expected (leftChunk,rightChunk) pair at: "+chunks);
        
        r.leftChk=chunks.substr(1,p-1);
        r.rightChk=chunks.substr(p+1,chunks.size()-p-2);
        
        // check if the chunk labels carried extra lemma/form/class/tag
        // conditions and separate them if that's the case.
        extract_conds(r.leftChk,r.leftConds);
        extract_conds(r.rightChk,r.rightConds);
        
        TRACE(3,L"Loaded rule: [line "+util::int2wstring(r.line)+L"] ");
        chgram[make_pair(r.leftChk,r.rightChk)].push_back(r);

        break;
      }

      default: break;
      }
    }

    cfg.close();

    TRACE(1,L"tree completer successfully created");
  }


  //-- tracing ----------------------
  void PrintTree(parse_tree::iterator n, int k, int depth) {
    parse_tree::sibling_iterator d;
  
    wcerr<<wstring(depth*2,L' '); 
    if (n->num_children()==0) { 
      if (n->info.is_head()) { wcerr<<L"+";}
      const word & w=n->info.get_word();
      wcerr<<L"("<<w.get_form()<<L" "<<w.get_lemma(k)<<L" "<<w.get_tag(k);
      wcerr<<L")"<<endl;
    }
    else { 
      if (n->info.is_head()) { wcerr<<L"+";}
      wcerr<<n->info.get_label()<<L"_[ "<<endl;
      for (d=n->sibling_begin(); d!=n->sibling_end(); ++d)
        PrintTree(d, k, depth+1);
      wcerr<<wstring(depth*2,L' ')<<L"]"<<endl;
    }
  }
  //--
  //-- tracing ----------------------
  void PrintDepTree (dep_tree::const_iterator n, int depth) {
    dep_tree::const_sibling_iterator d, dm;
    int last, min;
    bool trob;

    wcout << wstring (depth*2, ' ');

    parse_tree::const_iterator pn = n->info.get_link();
    wcout<<pn->info.get_label(); 
    wcout<<L"/" << n->info.get_label() << L"/";

    const word & w = n->info.get_word();
    wcout << L"(" << w.get_form() << L" " << w.get_lemma() << L" " << w.get_tag () << L")";
  
    if (n->num_children () > 0) {
      wcout << L" [" << endl;
    
      // Print Nodes
      for (d = n->sibling_begin (); d != n->sibling_end (); ++d)
        if (!d->info.is_chunk ())
          PrintDepTree (d, depth + 1);
    
      // print CHUNKS (in order)
      last = 0;
      trob = true;
      while (trob) {
        // while an unprinted chunk is found look, for the one with lower chunk_ord value
        trob = false;
        min = 9999;
        for (d = n->sibling_begin (); d != n->sibling_end (); ++d) {
          if (d->info.is_chunk ()) {
            if (d->info.get_chunk_ord () > last
                and d->info.get_chunk_ord () < min) {
              min = d->info.get_chunk_ord ();
              dm = d;
              trob = true;
            }
          }
        }
        if (trob)
          PrintDepTree (dm, depth + 1);
        last = min;
      }
    
      wcout << wstring (depth * 2, ' ') << L"]";
    }
    wcout << endl;
  }


  void completer::load_classes(const wstring &vclass, const wstring &vlemma,
                               const wstring &path, set<wstring> &cls) {

    // Class defined in a file, load it.
    if (vlemma[0]==L'"' && vlemma[vlemma.size()-1]==L'"') {

      wstring fname= util::absolute(vlemma,path);  // if relative, use main file path as base
    
      wifstream fclas;
      util::open_utf8_file(fclas,fname);
      if (fclas.fail()) ERROR_CRASH(L"Cannot open word class file "+fname);
    
      wstring line,lem;
      while (getline(fclas,line)) {
        if (line.empty() or line[0]==L'%') continue;

        wistringstream sline;
        sline.str(line);
        sline>>lem;
        cls.insert(vclass+L"#"+lem);
      }
      fclas.close();
    }

    // Enumerated class, just store the pair
    else 
      cls.insert(vclass+L"#"+vlemma);
  
  }

  ///////////////////////////////////////////////////////////////
  /// Complete a partial parse tree.
  ///////////////////////////////////////////////////////////////

  parse_tree completer::complete(parse_tree &tr, const wstring & startSymbol, dep_txala_status *st) const {

    TRACE(3,L"---- COMPLETING chunking to get a full parse tree ----");

    // we only complete trees with the fake start symbol set by the chart_parser
    if (tr.begin()->info.get_label() != startSymbol) return tr;  

    int maxchunk = tr.num_children();
    int nchunk=1;
  
    vector<parse_tree *> trees;
  
    for(parse_tree::sibling_iterator ichunk=tr.sibling_begin(); ichunk!=tr.sibling_end(); ++ichunk,++nchunk) {
      TRACE(4,L"Creating empty tree");

      parse_tree * mtree = new parse_tree(ichunk);
      if (ichunk->num_children()==0) {
        // the chunk is a leaf. Create a non-terminal node to ease 
        // the job in case no rules are found
        node nod(ichunk->info.get_label());
        nod.set_chunk(true);
        nod.set_head(false);
        nod.set_node_id(ichunk->info.get_node_id()+L"x");
        parse_tree *taux=new parse_tree(nod);
        // hang the original node under the new one.
        mtree->info.set_head(true);
        taux->hang_child(*mtree);
        // use the new tree
        mtree=taux;
      }

      mtree->info.set_chunk(nchunk);
      trees.push_back(mtree);
      TRACE(4,L"    Done");
    }
  
    // Apply as many rules as number of chunks minus one (since each rule fusions two chunks in one)
    for (nchunk=0; nchunk<maxchunk-1; ++nchunk) {
    
      if (traces::TraceLevel>=2) {
        TRACE(2,L"");
        TRACE(2,L"REMAINING CHUNKS:");
        vector<parse_tree *>::iterator vch; 
        int p=0;
        for (vch=trees.begin(); vch!=trees.end(); vch++) {
          TRACE(2,L"    chunk.num="+util::int2wstring((*vch)->info.get_chunk_ord())+L"  (trees["+util::int2wstring(p)+L"]) \t"+(*vch)->info.get_label());
          p++;
        }
      }

      TRACE(3,L"LOOKING FOR BEST APPLICABLE RULE");
      size_t chk=0;
      completerRule bestR = find_grammar_rule(trees, chk, st);
      int best_prio= bestR.weight;
      size_t best_pchunk = chk;

      chk=1;
      while (chk<trees.size()-1) {
        completerRule r = find_grammar_rule(trees, chk, st);

        if ( (r.weight==best_prio && chk<best_pchunk) || (r.weight>0 && r.weight<best_prio) || (best_prio<=0 && r.weight>best_prio) ) {
          best_prio = r.weight;
          best_pchunk = chk;
          bestR = r;
        }
      
        chk++;
      }
    
      TRACE(2,L"BEST RULE SELECTED. Apply rule [line "+util::int2wstring(bestR.line)+L"] to chunk trees["+util::int2wstring(best_pchunk)+L"]");
    
      parse_tree * resultingTree=applyRule(bestR, best_pchunk, trees[best_pchunk], trees[best_pchunk+1], st);

      TRACE(2,L"Rule applied - Erasing chunk in trees["+util::int2wstring(best_pchunk+1)+L"]");
      trees[best_pchunk]=resultingTree;
      trees[best_pchunk+1]=NULL;     
      vector<parse_tree*>::iterator end = remove (trees.begin(), trees.end(), (parse_tree*)NULL);    
      trees.erase (end, trees.end());    

      // clear rule appliactions to start fresh at next iteration
      st->last.clear();
    }
  
    // rebuild node index with new iterators, maintaining id's
    parse_tree ret_val = (*trees[0]); 
    delete trees[0];
    ret_val.rebuild_node_index();
    return (ret_val);
  }

  //---------- completer private functions 




  ///////////////////////////////////////////////////////////////
  /// Separate extra lemma/form/class conditions from the chunk label
  ///////////////////////////////////////////////////////////////

#define closing(x) (x==L'('?L")":(x==L'<'?L">":(x==L'{'?L"}":(x==L'['?L"]":L""))))

  void completer::extract_conds(wstring &chunk, matching_condition &cond) const {

    wstring seen=L"";
    wstring con=L"";
    cond.attrs.clear();

    cond.neg = false;
    if (chunk[0]==L'~') {
      cond.neg=true;
      chunk=chunk.substr(1);  
    }

    size_t p=chunk.find_first_of(L"<([{");
    if (p==wstring::npos) {
      // If the label has no pairs of "<>" "()" or "[]", we're done
      cond.label = chunk;
    }
    else  {
      // locate start of first pair, and separate chunk label
      con = chunk.substr(p);
      chunk = chunk.substr(0,p);
      cond.label = chunk;

      // process pairs
      p=0;
      while (p!=wstring::npos) {

        wstring close=closing(con[p]);
        size_t q=con.find_first_of(close);
        if (q==wstring::npos) {
          WARNING(L"Missing closing "+close+L" in dependency rule. All conditions ignored.");
          cond.attrs.clear();
          return;
        }
      
        // check for duplicates
        if (seen.find(con[p])!=wstring::npos) {
          WARNING(L"Duplicate bracket pair "+con.substr(p,1)+close+L" in dependency rule. All conditions ignored.");
          cond.attrs.clear();
          return;
        }

        // add the condition to the list
        matching_attrib ma;
        ma.type=con.substr(p,1);

        // store original condition string
        ma.value = con.substr(p,q-p+1);
        // if PoS tag condition, compile as regex (without {} brackets)
        if (ma.type==L"{") ma.re = freeling::regexp(con.substr(p+1,q-p-1));     
        // add to list of attribs
        cond.attrs.push_back(ma);

        // remember this type already appeared
        seen = seen + con[p];

        // find start of next pair, if any
        p=con.find_first_of(L"<([{",q);
      }
    }
  }


  ///////////////////////////////////////////////////////////////
  /// Check if the current context matches the one specified 
  /// in the given rule.
  ///////////////////////////////////////////////////////////////

#define LEFT 1
#define RIGHT 2
#define first_cond(d) (d==LEFT? conds.size()-1 : 0)
#define first_chk(d)  (d==LEFT? chk-1  : chk+2)
#define last(i,v,d)   (d==LEFT? i<0    : i>=(int)v.size())
#define next(i,d)     (d==LEFT? i-1    : i+1);
#define prev(i,d)     (d==LEFT? i+1    : i-1);

  bool completer::match_side(const int dir, const vector<parse_tree *> &trees, const size_t chk, const vector<matching_condition> &conds) const {

    TRACE(4,L"        matching "+wstring(dir==LEFT?L"LEFT":L"RIGHT")+L" context.  chunk position="+util::int2wstring(chk));
  
    // check whether left/right context matches
    bool match=true;
    int j=first_cond(dir); // condition index
    int k=first_chk(dir);  // chunk index
    while ( !last(j,conds,dir) && match ) {
      TRACE(4,L"        condition.idx j="+util::int2wstring(j)+L"   chunk.idx k="+util::int2wstring(k));

      if (last(k,trees,dir) && conds[j].label!=L"OUT") {
        // fail if context is shorter than rule (and the condition is not OUT-of-Bounds)
        match=false; 
      }
      else if (!last(k,trees,dir)) {
        bool b = (conds[j].label!=L"*") && (conds[j].label==L"?" || match_condition(trees[k],conds[j]));
        TRACE(4,L"        conds[j]="+conds[j].label+L" trees[k]="+trees[k]->begin()->info.get_label());

        if ( !b && conds[j].label==L"*" ) {
          // let the "*" match any number of items, looking for the next condition
          TRACE(4,L"        matching * wildcard.");

          j=next(j,dir);
          if (conds[j].label==L"OUT") // OUT after a wildcard will always match
            b=true;
          else {
            while ( !last(k,trees,dir) && !b ) {
              b = (conds[j].label==L"?") || match_condition(trees[k],conds[j]);
              TRACE(4,L"           j="+util::int2wstring(j)+L"  k="+util::int2wstring(k));
              TRACE(4,L"           conds[j]="+conds[j].label+L" trees[k]="+trees[k]->begin()->info.get_label()+L"   "+(b?wstring(L"matched"):wstring(L"no match")));
              k=next(k,dir);
            }
          
            k=prev(k,dir);        
          }
        }

        match = b;
      }

      k=next(k,dir);
      j=next(j,dir);
    }
  
    return match;  
  }


  ///////////////////////////////////////////////////////////////
  /// Check if the current context matches the one specified 
  /// in the given rule.
  ///////////////////////////////////////////////////////////////

  bool completer::matching_context(const vector<parse_tree *> &trees, const size_t chk, const completerRule &r) const {  
    // check whether the context matches      
    bool match = match_side(LEFT, trees, chk, r.leftContext) && match_side(RIGHT, trees, chk, r.rightContext);

    // apply negation if necessary
    if (r.context_neg) match = !match;

    TRACE(4,L"        Context "+wstring(match?L"matches":L"does NOT match"));
    return match;
  }


  ///////////////////////////////////////////////////////////////
  /// check if the extra lemma/form/class conditions are satisfied.
  ///////////////////////////////////////////////////////////////

  bool completer::match_condition(parse_tree::iterator chunk, const matching_condition &cond) const {
    
    bool ok;
    // if labels don't match, forget it.
    if (chunk->begin()->info.get_label() != cond.label) ok =false;
    
    // if no extra attributes, we're done.
    else if (cond.attrs.empty())  ok = true;
    
    else { 
      // dive into the tree to locate the head word.
      parse_tree::iterator head = chunk->begin();
      // while we don't reach a leaf
      while (head->num_children()>0) {
        // locate the head children..
        parse_tree::sibling_iterator s;
        for (s=head->sibling_begin(); s!=head->sibling_end() && !s->info.is_head(); ++s);    
        if (s==head->sibling_end())
          WARNING(L"NO HEAD Found!!! Check your chunking grammar and your dependency-building rules.");
      
        // ...and get down to it
        head=s;
      }
    
      // get word if head leaf node.
      const word & w = head->info.get_word();

      ok = true;    
      // check it satisfies all the condition attributes
      for (list<matching_attrib>::const_iterator c=cond.attrs.begin(); ok && c!=cond.attrs.end(); c++) {
        TRACE(4,L"        matching condition attr "+c->value+L" with word ("+w.get_form()+L","+w.get_lemma()+L","+w.get_tag()+L")");
        switch (c->type[0]) {
        case L'<': ok = (L"<"+w.get_lemma()+L">" == c->value); break;   
        case L'(': ok = (L"("+w.get_form()+L")" == c->value); break;
        case L'{': ok = c->re.search(w.get_tag()); break;
        case L'[': {
          wstring vclass=c->value.substr(1,c->value.size()-2);
          ok = (wordclasses.find(vclass+L"#"+w.get_lemma()) != wordclasses.end());        
          TRACE(4,L"        CLASS: "+c->value+wstring(ok?L" matches lemma":L" does NOT match lemma"));
          break;
        }    
        default: break; // should never get this far
        }
      }
    }

    // apply negation if the condition has one.
    if (cond.neg) ok = not ok;
  
    TRACE(4,L"            condition "+wstring(ok?L"match":L"does NOT match"));
    return ok;
  
  }    
  
  ///////////////////////////////////////////////////////////////
  /// check if the operation is executable (for last_left/last_right cases)
  ///////////////////////////////////////////////////////////////

  bool completer::matching_operation(const vector<parse_tree *> &trees, const size_t chk, const completerRule &r, dep_txala_status *st) const {

    // "top" operations are always feasible
    if (r.operation != L"last_left" && r.operation!=L"last_right" && r.operation!=L"cover_last_left") 
      return true;

    // "last_X" operations, require checking for the condition node
    size_t t=0;
    if (r.operation==L"last_left" || r.operation==L"cover_last_left") t=chk;
    else if (r.operation==L"last_right") t=chk+1;

    // build id to store that this is the application of rule num "r.line" to chunk number "chk"
    wstring aid = util::int2wstring(r.line)+L":"+util::int2wstring(chk);

    // locate last_left/right matching node 
    st->last[aid] = trees[t]->end();
    parse_tree::iterator i;
    for (i=trees[t]->begin(); i!=trees[t]->end(); ++i) {
      TRACE(5,L"           matching operation: "+r.operation+L". Rule expects "+r.newNode1+L", node is "+i->info.get_label());
      if (match_condition(i,r.matchingCond)) 
        st->last[aid] = i;  // remember node location in case the rule is finally selected.
    }

    TRACE(4,L"        Operation "+wstring(st->last[aid]!=trees[t]->end()?L"matches":L"does NOT match"));
    return st->last[aid]!=trees[t]->end();
  }


  ///////////////////////////////////////////////////////////////
  /// Find out if currently active flags enable the given rule
  ///////////////////////////////////////////////////////////////

  bool completer::enabled_rule(const completerRule &r, dep_txala_status *st) const {
   
    // if the rule is always-enabled, ignore everthing else
    if (r.enabling_flags.find(L"-") != r.enabling_flags.end()) 
      return true;

    // look through the rule enabling flags, to see if any is active.
    bool found=false;
    set<wstring>::const_iterator x;
    for (x=r.enabling_flags.begin(); !found && x!=r.enabling_flags.end(); x++)
      found = (st->active_flags.find(*x)!=st->active_flags.end());
  
    return found;
  }

  ///////////////////////////////////////////////////////////////
  /// Look for a completer grammar rule matching the given
  /// chunk in "chk" position of "trees" and his right-hand-side mate.
  ///////////////////////////////////////////////////////////////

  completerRule completer::find_grammar_rule(const vector<parse_tree *> &trees, const size_t chk, dep_txala_status *st) const {

    wstring leftChunk = trees[chk]->begin()->info.get_label();
    wstring rightChunk = trees[chk+1]->begin()->info.get_label();
    TRACE(3,L"  Look up rule for: ("+leftChunk+L","+rightChunk+L")");

    // find rules matching the chunks
    map<pair<wstring,wstring>,list<completerRule> >::const_iterator r;
    r = chgram.find(make_pair(leftChunk,rightChunk));
 
    list<completerRule>::const_iterator i;  
    list<completerRule>::const_iterator best;
    int bprio= -1;
    bool found=false;
    if (r != chgram.end()) { 

      // search list of candidate rules for any rule matching conditions, context, and flags
      for (i=r->second.begin(); i!=r->second.end(); i++) {

        TRACE(4,L"    Checking candidate: [line "+util::int2wstring(i->line)+L"] ");

        // check extra conditions on chunks and context
        if (enabled_rule(*i,st)  
            && match_condition(trees[chk]->begin(),i->leftConds) 
            && match_condition(trees[chk+1]->begin(),i->rightConds) 
            && matching_context(trees,chk,*i)
            && matching_operation(trees,chk,*i,st)) {
     
          if (bprio == -1 || bprio>i->weight) {
            found = true;
            best=i;
            bprio=i->weight;
          }

          TRACE(3,L"    Candidate: [line "+util::int2wstring(i->line)+L"] -- MATCH");
        }
        else  {
          TRACE(3,L"    Candidate: [line "+util::int2wstring(i->line)+L"] -- no match");
        }
      }
    }

    if (found) 
      return (*best);
    else {
      TRACE(3,L"    NO matching candidates found, applying default rule.");    
      // Default rule: top_left, no relabel
      return completerRule(L"-",L"-",L"top_left");
    }
  
  }


  ///////////////////////////////////////////////////////////////
  /// apply a tree completion rule
  ///////////////////////////////////////////////////////////////

  parse_tree * completer::applyRule(const completerRule & r, int chk, parse_tree * chunkLeft, parse_tree * chunkRight, dep_txala_status *st) const {
  
    // toggle necessary flags on/off
    set<wstring>::const_iterator x;
    for (x=r.flags_toggle_on.begin(); x!=r.flags_toggle_on.end(); x++) 
      st->active_flags.insert(*x);
    for (x=r.flags_toggle_off.begin(); x!=r.flags_toggle_off.end(); x++) 
      st->active_flags.erase(*x);

    // build id to retrieve that this is the application of rule num "r.line" to chunk number "chk"
    wstring aid = util::int2wstring(r.line)+L":"+util::int2wstring(chk);

    // hang left tree under right tree root
    if (r.operation==L"top_right") {
      TRACE(3,L"Applying rule: Insert left chunk under top_right");
      // Right is head
      chunkLeft->begin()->info.set_head(false);      
      chunkRight->begin()->info.set_head(true);

      // change node labels if required
      if (r.newNode1!=L"-") {
        TRACE(3,L"    ... and relabel left chunk (child) as "+r.newNode1);
        chunkLeft->begin()->info.set_label(r.newNode1);      
      }
      if (r.newNode2!=L"-") {
        TRACE(3,L"    ... and relabel right chunk (parent) as "+r.newNode2);
        chunkRight->begin()->info.set_label(r.newNode2);   
      }   

      // insert Left tree under top node in Right
      chunkRight->hang_child(*chunkLeft,false);
      return chunkRight;
    }
  
    // hang right tree under left tree root and relabel root
    else if (r.operation==L"top_left") {
      TRACE(3,L"Applying rule: Insert right chunk under top_left");
      // Left is head
      chunkRight->begin()->info.set_head(false);
      chunkLeft->begin()->info.set_head(true);

      // change node labels if required
      if (r.newNode1!=L"-") {
        TRACE(3,L"    ... and relabel left chunk (parent) as "+r.newNode1);
        chunkLeft->begin()->info.set_label(r.newNode1);      
      }   
      if (r.newNode2!=L"-") {
        TRACE(3,L"    ... and relabel right chunk (child) as "+r.newNode2);
        chunkRight->begin()->info.set_label(r.newNode2);   
      }

      // insert Right tree under top node in Left
      chunkLeft->hang_child(*chunkRight);
      return chunkLeft;
    }

    // hang right tree under last node in left tree
    else if (r.operation==L"last_left") {
      TRACE(3,L"Applying rule: Insert right chunk under last_left with label "+r.matchingCond.label);
      // Left is head, so unmark Right as head.
      chunkRight->begin()->info.set_head(false); 
      TRACE(4,L"recovering last right match for rule "+util::int2wstring(r.line)+L" application "+aid);
      // obtain last node with given label in Left tree
      // We stored it in the rule when checking for its applicability.
      parse_tree::iterator p=st->last[aid];
      TRACE(4,L"   node recovered is: ");
      TRACE(4,L"     "+p->info.get_label());    // hang Right tree under last node in Left tree
      p->hang_child(*chunkRight); 
      return chunkLeft;
    }
  
    // hang left tree under 'last' node in right tree
    else if (r.operation==L"last_right") {
      TRACE(3,L"Applying rule: Insert left chunk under last_right with label "+r.newNode1);
      // Right is head, so unmark Left as head.
      chunkLeft->begin()->info.set_head(false); 
      // obtain last node with given label in Right tree
      // We stored it in the rule when checking for its applicability.
      parse_tree::iterator p=st->last[aid];
      // hang Left tree under last node in Right tree
      p->hang_child(*chunkLeft,false); 
      return chunkRight;
    }

    // hang right tree where last node in left tree is, and put the later under the former
    else if (r.operation==L"cover_last_left") {
      TRACE(3,L"Applying rule: Insert right chunk to cover_last_left with label "+r.newNode1);
      // Right will be the new head
      chunkRight->begin()->info.set_head(true); 
      chunkLeft->begin()->info.set_head(false); 
      // obtain last node with given label in Left tree
      // We stored it in the rule when checking for its applicability.
      parse_tree::iterator last=st->last[aid];
      parse_tree::iterator parent = last->get_parent();
      // put last_left tree under Right tree (removing it from its original place)
      chunkRight->hang_child(*last); 
      // put chunkRight under the same parent that last_left
      parent->hang_child(*chunkRight);
      return chunkLeft;
    }

    else {
      ERROR_CRASH(L"Internal Error unknown rule operation type: "+r.operation);
      return NULL; // avoid compiler warnings
    }
  
  }


  //---------- Class depLabeler ----------------------------------

  ///////////////////////////////////////////////////////////////
  /// Constructor: create dependency parser
  ///////////////////////////////////////////////////////////////

  depLabeler::depLabeler(const wstring &filename) : semdb(NULL) {

    wstring path=filename.substr(0,filename.find_last_of(L"/\\")+1);
    wstring sdb;

    enum sections {CLASS,GRLAB,SEMDB};
    config_file cfg(true,L"%");  
    cfg.add_section(L"CLASS",CLASS);
    cfg.add_section(L"GRLAB",GRLAB);
    cfg.add_section(L"SEMDB",SEMDB);

    if (not cfg.open(filename))
      ERROR_CRASH(L"Error opening labeler file "+filename);

    wordclasses.clear();
    wstring line;
    while (cfg.get_content_line(line)) {

      int lnum = cfg.get_line_num();
    
      switch (cfg.get_section()) {

      case CLASS : {
        // load CLASS section
        wstring vclass, vlemma;
        wistringstream sin;  sin.str(line);
        sin>>vclass>>vlemma;
        completer::load_classes(vclass, vlemma, path, wordclasses);
        break;
      }
    
      case GRLAB: {
        ////// load GRLAB section defining labeling rules
        check_and * expr= new check_and();
        
        // Read first word in line
        wstring s;
        wistringstream sin;  sin.str(line);
        sin>>s;
        if (s==L"UNIQUE") {
          // "UNIQUE" key found, add labels in the line to the set
          wstring lab; 
          while (sin>>lab) unique.insert(lab);
        }
        else {
          // not "UNIQUE" key, it is a normal rule.
          ruleLabeler r;
          r.line=lnum;
          r.ancestorLabel=s;  // we had already read the first word.
          sin>>r.label;       // second word is the label
          
          TRACE(4,L"RULE FOR:"+r.ancestorLabel+L" -> "+r.label);
          wstring condition;
          while (sin>>condition) 
            expr->add(build_expression(condition));
          
          r.re=expr;
          rules[r.ancestorLabel].push_back(r);      
        }

        break;
      }
        
      case SEMDB: {
        /// load SEMDB section
        wistringstream sin;  sin.str(line);
        sin>>sdb;
        sdb = util::absolute(sdb,path); 
        if ( not sdb.empty() ) {
          delete semdb; // free memory, just in case
          semdb = new semanticDB(sdb);
          TRACE(3,L"depLabeler loaded SemDB");
        }
        break;
      }

      default: break;
      }
    }
    cfg.close();


    TRACE(1,L"depLabeler successfully created");
  }

  ///////////////////////////////////////////////////////////////
  /// Constructor: create dependency parser
  ///////////////////////////////////////////////////////////////

  depLabeler::~depLabeler() {
    delete semdb;
  }


  ///////////////////////////////////////////////////////////////
  /// Constructor private method: parse conditions and build rule expression
  ///////////////////////////////////////////////////////////////

  rule_expression* depLabeler::build_expression(const wstring &condition) const {

    rule_expression *re=NULL;
    // parse condition and obtain components
    wstring::size_type pos=condition.find(L'=');
    wstring::size_type pdot=condition.rfind(L'.',pos);
    if (pos!=wstring::npos && pdot!=wstring::npos) {
      // Disassemble the condition. E.g. for condition "p.class=mov" 
      // we get: node="p";  func="class"; value="mov"; negated=false
      bool negated = (condition[pos-1]==L'!');
      wstring node=condition.substr(0,pdot);
      wstring func=condition.substr(pdot+1, pos-pdot-1-(negated?1:0) );
      wstring value=condition.substr(pos+1);
      TRACE(4,L"  adding condition:("+node+L","+func+L","+value+L")");    

      // check we do not request impossible things
      if (semdb==NULL && (func==L"tonto" || func==L"semfile" || func==L"synon" || func==L"asynon")) {
        ERROR_CRASH(L"Semantic function '"+func+L"' was used in labeling rules, but no previous <SEMDB> section was found. Make sure <SEMDB> section is defined before <GRLAB> section.");
      }

      // if the check is negated and we have As or Es, we must invert them
      if (negated) {
        if (node[0]==L'A') node[0]=L'E';
        else if (node[0]==L'E') node[0]=L'A';
      }

      // create rule checker for function requested in condition
      if (func==L"label")        re=new check_category(node,value);
      else if (func==L"side")    re=new check_side(node,value);
      else if (func==L"lemma")   re=new check_lemma(node,value);
      else if (func==L"pos")     re=new check_pos(node,value);
      else if (func==L"class")   re=new check_wordclass(node,value,&wordclasses);
      else if (func==L"tonto")   re=new check_tonto(*semdb,node,value);
      else if (func==L"semfile") re=new check_semfile(*semdb,node,value);
      else if (func==L"synon")   re=new check_synon(*semdb,node,value);
      else if (func==L"asynon")  re=new check_asynon(*semdb,node,value);
      else 
        WARNING(L"Error reading dependency rules. Ignored unknown function "+func);
    
      if (negated) re=new check_not(re);
    } 
    else 
      WARNING(L"Error reading dependency rules. Ignored incorrect condition "+condition);

    return re;
  }



  ///////////////////////////////////////////////////////////////
  /// Label nodes in a depencendy tree. (Initial call)
  ///////////////////////////////////////////////////////////////

  void depLabeler::label(dep_tree * dependency) const {
    TRACE(2,L"------ LABELING Dependences ------");
    dep_tree::iterator d=dependency->begin(); 
    d->info.set_label(L"top");
    label(dependency, d);
  }


  ///////////////////////////////////////////////////////////////
  /// Label nodes in a depencendy tree. (recursive)
  ///////////////////////////////////////////////////////////////

  void depLabeler::label(dep_tree* dependency, dep_tree::iterator ancestor) const {

    dep_tree::sibling_iterator d,d1;

    // there must be only one top 
    for (d=ancestor->sibling_begin(); d!=ancestor->sibling_end(); ++d) {
          
      ///const string ancestorLabel = d->info.get_dep_result();
      const wstring ancestorLabel = ancestor->info.get_link()->info.get_label();
      TRACE(2,L"Labeling dependency: "+d->info.get_link()->info.get_label()+L" --> "+ancestorLabel);
    
      map<wstring, list <ruleLabeler> >::const_iterator frule=rules.find(ancestorLabel);
      if (frule!=rules.end()) {
        list<ruleLabeler>::const_iterator rl=frule->second.begin();
        bool found=false;

        while (rl!=frule->second.end() && !found) {

          TRACE(3,L"  Trying rule: [line "+util::int2wstring(rl->line)+L"] ");
          bool skip=false;
          // if the label is declared as unique and a sibling already has it, skip the rule.
          if (unique.find(rl->label)!=unique.end())
            for (d1=ancestor->sibling_begin(); d1!=ancestor->sibling_end() && !skip; ++d1)
              skip = (rl->label == d1->info.get_label());
        
          if (!skip) {
            found = (rl->check(ancestor,d)); 
            if (found) { 
              d->info.set_label(rl->label);
              TRACE(3,L"      [line "+util::int2wstring(rl->line)+L"] "+rl->ancestorLabel+L" "+rl->label+L" -- rule matches!");
              TRACE(2,L"      RULE APPLIED. Dependence labeled as "+rl->label);
            }
            else {
              TRACE(3,L"      [line "+util::int2wstring(rl->line)+L"] "+rl->ancestorLabel+L" "+rl->label+L" -- no match");
            }
          }
          else {
            TRACE(3,L"      RULE SKIPPED -- Unique label "+rl->label+L" already present.");
          }

          ++rl;
        }
       
        if (!found) {d->info.set_label(L"modnomatch");}
      }
      else { 
        d->info.set_label(L"modnorule");
      }     
   
      // Recursive call
      label(dependency, d);
    }
  }


  //---------- Class dep_txala ----------------------------------

  ///////////////////////////////////////////////////////////////
  /// constructor. Load a dependecy rule file.
  ///////////////////////////////////////////////////////////////

  dep_txala::dep_txala(const wstring & fullgram, const wstring & startSymbol) : comp(fullgram), labeler(fullgram), start(startSymbol) {}


  ///////////////////////////////////////////////////////////////
  /// Enrich given sentence with a depenceny tree.
  ///////////////////////////////////////////////////////////////

  void dep_txala::analyze(sentence &s) const {

    // parse each of k-best tag sequences
    for (unsigned int k=0; k<s.num_kbest(); k++) {

      dep_txala_status *st = new dep_txala_status();
      st->active_flags.insert(L"INIT");
      s.set_processing_status((processor_status*)st);

      // get chunker output
      parse_tree buff = s.get_parse_tree(k);
      // complete it into a full parsing tree
      parse_tree ntr = comp.complete(buff,start,st);
      // store complete tree in the sentence
      s.set_parse_tree(ntr,k);    
  
      /// PrintTree(ntr.begin(),k,0); // debugging
    
      // We need to keep the original parse tree as we are storing iterators!!
      dep_tree* deps = dependencies(s.get_parse_tree(k).begin(),s.get_parse_tree(k).begin());

      // Set labels on the dependencies
      labeler.label(deps);
    
      // store the tree
      s.set_dep_tree(*deps,k);
      // PrintDepTree(s.get_dep_tree().begin(),0); // debugging

      delete(deps);
      s.clear_processing_status();
    }

  }


  //---------- dep_txala private functions 

  ///////////////////////////////////////////////////////////////
  /// Obtain a depencendy tree from a parse tree.
  ///////////////////////////////////////////////////////////////

  dep_tree * dep_txala::dependencies(parse_tree::iterator tr, parse_tree::iterator link) const {

    dep_tree * result;

    if (tr->num_children() == 0) { 
      // direct case. Leaf node, just build and return a one-node dep_tree.
      depnode d(tr->info); 
      d.set_link(link);
      result = new dep_tree(d);
    }
    else {
      // Recursive case. Non-leaf node. build trees 
      // for all children and hang them below the head

      // locate head child
      parse_tree::sibling_iterator head;
      parse_tree::sibling_iterator k;
      for (k=tr->sibling_begin(); k!=tr->sibling_end() && !k->info.is_head(); ++k);
      if (k==tr->sibling_end()) {
        WARNING(L"NO HEAD Found!!! Check your chunking grammar and your dependency-building rules.");
        k=tr->sibling_begin();
      }
      head = k;
    
      // build dep tree for head child
      if (!tr->info.is_head()) link=tr;
      result = dependencies(head,link);
    
      // Build tree for each non-head child and hang it under the head.
      // We maintain the original sentence order (not really necessary,
      // but trees are cuter this way...)

      // children to the left of the head
      k=head; ++k;
      while (k!=tr->sibling_end()) { 
        if (k->info.is_head()) WARNING(L"More than one HEAD detected. Only first prevails.");
        dep_tree *dt = dependencies(k,k);
        result->hang_child(*dt);  // hang it as last child
        ++k;
      }
      // children to the right of the head
      k=head; --k;
      while (k!=tr->sibling_rend()) { 
        if (k->info.is_head()) WARNING(L"More than one HEAD detected. Only first prevails.");
        dep_tree *dt = dependencies(k,k);
        result->hang_child(*dt,false);   // hang it as first child
        --k;
      }
    }

    // copy chunk information from parse tree
    result->info.set_chunk(tr->info.get_chunk_ord());  
    return (result);
  }

} // namespace

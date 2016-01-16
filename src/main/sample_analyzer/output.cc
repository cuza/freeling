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


//////////////////////////////////////////////////////////
///  Auxiliary functions to print several analysis results
//////////////////////////////////////////////////////////

#include "output.h"

using namespace std;


//---------------------------------------------
// Constructor
//---------------------------------------------

output::output(config *c) {
  cfg=c;
}

//---------------------------------------------
// Print senses information for an analysis
//---------------------------------------------

wstring output::outputSenses (const analysis & a) {

  wstring res;
  const list<pair<wstring,double> > & ls = a.get_senses ();
  if (ls.size () > 0) {
    if (cfg->SENSE_WSD_which == MFS)
      res = L" " + ls.begin()->first;
    else  // ALL or UKB specified
      res = L" " + util::pairlist2wstring (ls, L":", L"/");
  }
  else
    res = L" -";

  return res;
}


//---------------------------------------------
// print parse tree
//--------------------------------------------

void output::PrintTree (wostream &sout, parse_tree::const_iterator n, int depth, const document &doc) {

  parse_tree::const_sibling_iterator d;

  sout << wstring (depth * 2, ' ');  
  if (n->num_children () == 0) {
    if (n->info.is_head ()) sout << L"+";
    const word & w = n->info.get_word ();
    sout << L"(" << w.get_form() << L" " << w.get_lemma() << L" " << w.get_tag ();
    sout << outputSenses ((*w.selected_begin ()));
    sout << L")" << endl;
  }
  else {
    if (n->info.is_head ()) sout << L"+";

    sout<<n->info.get_label();
    if (cfg->COREF_CoreferenceResolution) {
      // Print coreference group, if needed.
      int ref = doc.get_coref_group(n->info.get_node_id());
      if (ref != -1 and n->info.get_label() == L"sn") sout<<L"(REF:" << ref <<L")";
    }
    sout << L"_[" << endl;

    for (d = n->sibling_begin (); d != n->sibling_end (); ++d) 
      PrintTree (sout, d, depth + 1, doc);
    sout << wstring (depth * 2, ' ') << L"]" << endl;
  }
}


//---------------------------------------------
// print dependency tree
//---------------------------------------------

void output::PrintDepTree (wostream &sout, dep_tree::const_iterator n, int depth, const document &doc) {
  dep_tree::const_sibling_iterator d, dm;
  int last, min, ref;
  bool trob;

  sout << wstring (depth*2, ' ');

  parse_tree::const_iterator pn = n->info.get_link();
  sout<<pn->info.get_label(); 
  ref = (cfg->COREF_CoreferenceResolution ? doc.get_coref_group(pn->info.get_node_id()) : -1);
  if (ref != -1 and pn->info.get_label() == L"sn") {
    sout<<L"(REF:" << ref <<L")";
  }
  sout<<L"/" << n->info.get_label() << L"/";

  const word & w = n->info.get_word();
  sout << L"(" << w.get_form() << L" " << w.get_lemma() << L" " << w.get_tag ();
  sout << outputSenses ((*w.selected_begin()));
  sout << L")";
  
  if (n->num_children () > 0) {
    sout << L" [" << endl;
    
    // Print Nodes
    for (d = n->sibling_begin (); d != n->sibling_end (); ++d)
      if (!d->info.is_chunk ())
        PrintDepTree (sout, d, depth + 1, doc);
    
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
        PrintDepTree (sout, dm, depth + 1, doc);
      last = min;
    }
    
    sout << wstring (depth * 2, ' ') << L"]";
  }
  sout << endl;
}


//---------------------------------------------
// print retokenization combinations for a word
//---------------------------------------------

list<analysis> output::printRetokenizable(wostream &sout, const list<word> &rtk, list<word>::const_iterator w, const wstring &lem, const wstring &tag) {
  
  list<analysis> s;
  if (w==rtk.end()) 
    s.push_back(analysis(lem.substr(1),tag.substr(1)));
      
  else {
    list<analysis> s1;
    list<word>::const_iterator w1=w; w1++;
    for (word::const_iterator a=w->begin(); a!=w->end(); a++) {
      s1=printRetokenizable(sout, rtk, w1, lem+L"+"+a->get_lemma(), tag+L"+"+a->get_tag());
      s.splice(s.end(),s1);
    }
  }
  return s;
}  


//---------------------------------------------
// print analysis for a word
//---------------------------------------------

void output::PrintWord (wostream &sout, const word &w, bool only_sel, bool probs) {
  word::const_iterator ait;

  word::const_iterator a_beg,a_end;
  if (only_sel) {
    a_beg = w.selected_begin();
    a_end = w.selected_end();
  }
  else {
    a_beg = w.analysis_begin();
    a_end = w.analysis_end();
  }

  for (ait = a_beg; ait != a_end; ait++) {
    if (ait->is_retokenizable ()) {
      const list <word> & rtk = ait->get_retokenizable();
      list <analysis> la=printRetokenizable(sout, rtk, rtk.begin(), L"", L"");
      for (list<analysis>::iterator x=la.begin(); x!=la.end(); x++) {
        sout << L" " << x->get_lemma() << L" " << x->get_tag();
        if (probs) sout << L" " << ait->get_prob()/la.size();
      }
    }
    else {
      sout << L" " << ait->get_lemma() << L" " << ait->get_tag ();
      if (probs) sout << L" " << ait->get_prob ();
    }

    if (cfg->SENSE_WSD_which != NONE)
      sout << outputSenses (*ait);
  }
}

//---------------------------------------------
// print obtained analysis.
//---------------------------------------------

void output::PrintResults (wostream &sout, list<sentence > &ls, const document &doc) {
  sentence::const_iterator w;
  list<sentence>::iterator is;
    
  for (is = ls.begin (); is != ls.end (); is++) {
    if (cfg->OutputFormat >= SHALLOW) {
      /// obtain parse tree and draw it at will
      switch (cfg->OutputFormat) {

        case SHALLOW:
        case PARSED: {
          parse_tree & tr = is->get_parse_tree ();
          PrintTree (sout, tr.begin (), 0, doc);
          sout << endl;
        }
        break;
   
        case DEP: {
          dep_tree & dep = is->get_dep_tree ();
          PrintDepTree (sout, dep.begin (), 0, doc);
        }
        break;
   
        default:   // should never happen
        break;
      }
    }
    else {
      for (w = is->begin (); w != is->end (); w++) {
        sout << w->get_form();
        if (cfg->PHON_Phonetics) sout<<L" "<<w->get_ph_form();
   
        if (cfg->OutputFormat == MORFO or cfg->OutputFormat == TAGGED) {
          if (cfg->TrainingOutput) {
            /// Trainig output: selected analysis (no prob) + all analysis (with probs)
            PrintWord(sout,*w,true,false);
            sout<<L" #";
            PrintWord(sout,*w,false,true);
          }
          else {
            /// Normal output: selected analysis (with probs)
            PrintWord(sout,*w,true,true);  
          }
        }

        sout << endl;   
      }
    }
    // sentence separator: blank line.
    sout << endl;
  }
}

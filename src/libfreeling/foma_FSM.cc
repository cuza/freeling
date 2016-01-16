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

#include <fstream>
#include <sstream>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/foma_FSM.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"FOMA_FSM"
#define MOD_TRACECODE ALTERNATIVES_TRACE


  ///////////////////////////////////////////////////////////////
  /// Create foma FSM from given text file
  ///////////////////////////////////////////////////////////////

  foma_FSM::foma_FSM(const wstring &fname, const wstring &mcost) {
  
    char saux[1024];  // auxiliar for string conversion
    set<wstring> alph;   // automata alphabet

    // load dictionary file and build a minimum FSA for that language

    if (fname.rfind(L".src")==fname.rfind(L".")) {  // build FSA from text dictionary

      // Create temp file with keys only, to please foma
      wstring formlist = util::new_tempfile_name()+L".src";
      TRACE(3,L"Creating form list in "+formlist);
      wofstream fform;
      util::open_utf8_file(fform,formlist);
      if (fform.fail()) ERROR_CRASH(L"Error creating temp file "+formlist);

      // Read text dictionary and dump keys into temp file.
      // Also use this loop to compute automata alphabet
      wifstream fabr;
      util::open_utf8_file(fabr,fname);
      if (fabr.fail()) ERROR_CRASH(L"Error opening file "+fname);    
      wstring line; 
      while (getline(fabr,line)) {
        // get key
        wistringstream sin; sin.str(line);
        wstring key; sin>>key;
        // add symbols to alphabet
        for (size_t i=0; i<key.size(); i++) alph.insert(wstring(1,key[i]));
        // add form to temp file for FOMA
        fform<<key<<endl;
      }
      fabr.close();
      fform.close();

      // convert tempfile name to char* for FOMA
      strcpy(saux,util::wstring2string(formlist).c_str());

      fsa = fsm_read_text_file(saux);     // create FSA
      fsa = fsm_minimize(fsa);  

      remove(saux);  // remove temp file
    }

    else if (fname.rfind(L".bin")==fname.rfind(L".")) {  // FSA already compiled

      // do not allow cost matrix with binary files (they should have cmatrix already compiled into)      
      if (not mcost.empty()) 
        ERROR_CRASH(L"Unexpected cost matrix given with binary FSA file.");

      fsa = fsm_read_binary_file(saux);
    }

    else 
      ERROR_CRASH(L"Unknown file extension for '"+fname+L". Expected '.src' or '.bin'");

    if (not mcost.empty()) { // Cost matrix provided

      if (alph.empty())  // mcost is only allowed with text dictionaries
        WARNING(L"Ignoring specified cost matrix, since automata file is binary.");

      else {        
        // remember size of the FSA alphabet 
        size_t alphsz = alph.size();
        
        // read cost matrix, and add any missing symbols to alphabet
        wifstream fabr;
        util::open_utf8_file(fabr,mcost);
        if (fabr.fail()) ERROR_CRASH(L"Error opening file "+mcost);
        wstring line;
        while (getline(fabr,line)) {
          if (line[0]==L' ') {
            list<pair<wstring,wstring> > p = freeling::util::wstring2pairlist<wstring,wstring>(line.substr(1),L":",L" ");
            for (list<pair<wstring,wstring> >::iterator k=p.begin(); k!=p.end(); k++) {
              alph.insert(k->first);
              alph.insert(k->second);
            }
          }
        }
        fabr.close();
        
        // if alphabet gained symbols from matrix, compose the automata with Sigma*,
        // to make sure the automata has all matrix symbols (even if it doesn't 
        // actually use them)
        if (alph.size() > alphsz) {
          wstring alphabet = L"[\"" + freeling::util::set2wstring(alph,L"\"|\"") + L"\"]*";
          TRACE(3,L"Composing with full alphabet: "+alphabet);
          strcpy(saux,util::wstring2string(alphabet).c_str());
          struct fsm *alf=fsm_parse_regex(saux);
          fsa = fsm_compose(fsa,alf);
        }
        
        // read cost matrix file into a string buffer.
        TRACE(3,L"Reading cost matrix: "+mcost);
        std::ifstream mc(util::wstring2string(mcost).c_str());
        std::stringstream buffer; buffer << mc.rdbuf();
        char *cms = new char[buffer.str().size()+1];
        strcpy(cms,buffer.str().c_str());
        
        // load cost matrix and associate it to the automata
        my_cmatrixparse(fsa,cms);
        delete[] cms;
      }
    }

    // initialize for minimum edit distance searches
    h_fsa = apply_med_init(fsa);

    // set search parameters
    apply_med_set_heap_max(h_fsa, 4194304);  // max heap to use (4MB)
    apply_med_set_med_limit(h_fsa, 20);      // max of matches to get
  }

  ///////////////////////////////////////////////////////////////
  /// Destructor, free foma structs
  ///////////////////////////////////////////////////////////////

  foma_FSM::~foma_FSM() {
    apply_med_clear(h_fsa); 
    free(fsa);
  }

  ///////////////////////////////////////////////////////////////
  /// Set maximum edit distance to retrieve
  ///////////////////////////////////////////////////////////////

  void foma_FSM::set_cutoff_threshold(int thr) {
    apply_med_set_med_cutoff(h_fsa, thr); // max distance for matches
  }

  ///////////////////////////////////////////////////////////////
  /// Set maximum number of matches to retrieve
  ///////////////////////////////////////////////////////////////

  void foma_FSM::set_num_matches(int max) {
    apply_med_set_med_limit(h_fsa, max);      // max of matches to get
  }

  ///////////////////////////////////////////////////////////////
  /// Set cost for basic SED operations to given value
  ///////////////////////////////////////////////////////////////

  void foma_FSM::set_basic_operation_cost(int cost) {
    cmatrix_init(fsa);
    cmatrix_default_insert(fsa,cost);
    cmatrix_default_delete(fsa,cost);
    cmatrix_default_substitute(fsa,cost);
  }

  ////////////////////////////////////////////////////////////////////////
  /// Use automata to obtain closest matches to given form, 
  /// adding them (and the distance) to given list.
  ////////////////////////////////////////////////////////////////////////

  void foma_FSM::get_similar_words(const wstring &form, list<pair<wstring,int> > &alts) const {

    TRACE(3,L"Copying to char buffer");
    // convert input const wstring to non-const char* to satisfy foma API
    char* search = new char[form.size()*sizeof(wchar_t)+1];
    strcpy(search,util::wstring2string(form).c_str());

    set<wstring> seen;  // avoid duplicates, keep only first occurrence (lowest cost)

    // get closest match for search form
    TRACE(3,L"Search closest match");
    char *result = apply_med(h_fsa, search);
    while (result) {
      wstring alt = util::string2wstring(string(result));
      // if result is new, add to list      
      if (seen.find(alt)==seen.end()) {  
        // it is no longer new
        seen.insert(alt);
        // get distance
        int c = apply_med_get_cost(h_fsa);
        // store alternative in list
        alts.push_back(make_pair(alt,c));
      }
    
      // Call with NULL on subsequent calls to get next alternatives
      result = apply_med(h_fsa, NULL);
    }

    free(result);
    delete[] search;
  }

} // namespace

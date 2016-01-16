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
#include <vector>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/dictionary.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/alternatives.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"ALTERNATIVES"
#define MOD_TRACECODE ALTERNATIVES_TRACE


  ///////////////////////////////////////////////////////////////
  ///  Create a alternatives module, loading dictionary and options
  ///////////////////////////////////////////////////////////////

  alternatives::alternatives(const std::wstring &altsFile) : CheckKnownTags(L"^$") {
 
    wstring path=altsFile.substr(0,altsFile.find_last_of(L"/\\")+1);
 
    // default
    DistanceThreshold=2;
    MaxSizeDiff=3;
    CheckUnknown=true;
    sed=NULL;

    enum sections {GENERAL,DISTANCE,TARGET};
    config_file cfg(true);
    cfg.add_section(L"General",GENERAL);
    cfg.add_section(L"Distance",DISTANCE);
    cfg.add_section(L"Target",TARGET);

    if (not cfg.open(altsFile))
      ERROR_CRASH(L"Error opening file "+altsFile);

    wstring dic_file, ph_file, cost_file;
    int dic_type=0;

    // load ortographic alternatives configuration
    wstring line;
    while (cfg.get_content_line(line)) {
      wistringstream sin;
      sin.str(line);
      wstring key,value;
      sin>>key>>value;

      switch (cfg.get_section()) {
        // Read <General> section contents
      case GENERAL: {
        if (key==L"Type") {
          if (freeling::util::lowercase(value)==L"orthographic") DistanceType=ORTHOGRAPHIC;
          else if (freeling::util::lowercase(value)==L"phonetic") DistanceType=PHONETIC;
          else ERROR_CRASH(L"Unexpected value '"+value+L"' for 'Type' in file "+altsFile);
        }
        else if (key==L"Dictionary") {
          if (dic_type!=0) 
            ERROR_CRASH(L"More than one Dictionary specified in file "+altsFile);
          dic_type = ORTHOGRAPHIC;
          dic_file = util::absolute(value,path); 
        }
        else if (key==L"PhoneticDictionary") {
          if (dic_type!=0) 
            ERROR_CRASH(L"More than one Dictionary specified in file "+altsFile);
          dic_type = PHONETIC;
          dic_file = util::absolute(value,path); 
        }
        else if (key==L"PhoneticRules") 
          ph_file = util::absolute(value,path); 
        else 
          WARNING(L"Ignoring unexpected line '"+line+L"' in file "+altsFile);
        break;
      }

        // Read <Distance> section contents
      case DISTANCE: {
        if (key==L"CostMatrix") 
          cost_file = util::absolute(value,path); 
        else if (key==L"Threshold") 
          DistanceThreshold = freeling::util::wstring2int(value);
        else if (key==L"MaxSizeDiff") 
          MaxSizeDiff = freeling::util::wstring2int(value);
        else 
          WARNING(L"Ignoring unexpected line '"+line+L"' in file "+altsFile);
        break;
      }

        // Read <Target> section contents
      case TARGET: {
        if (key==L"UnknownWords")  {
          value=util::lowercase(value);
          CheckUnknown = (value==L"yes" or value==L"y");
        }
        else if (key==L"KnownWords") {
          if (value!=L"none" and value!=L"no") CheckKnownTags = freeling::regexp(value);
        }
        else 
          WARNING(L"Ignoring unexpected line '"+line+L"' in file "+altsFile);
        break;
      }
      default: break;
      }
    }

    cfg.close();

    if (dic_type==0) ERROR_CRASH(L"No dictionary specified.");
    if (dic_type==PHONETIC and DistanceType!=PHONETIC) ERROR_CRASH(L"Only 'Type phonetic' is allowed with PhoneticDictionary.");
    if ((dic_type==PHONETIC) == (ph_file.empty())) ERROR_CRASH(L"PhoneticRules must be specified iff PhoneticDictionary is used.");

    /// Create phonetic transcriptor
    ph = NULL;
    if (not ph_file.empty()) {
      TRACE(3,L"Creating phonetic encoder");
      ph = new phonetics(ph_file);
    }

    wstring phonlist;
    if (DistanceType==PHONETIC) {

      wofstream fphon;
      if (dic_type==ORTHOGRAPHIC) {
        // if dictionary is not phonetic create temp file to encode a phonetic version
        phonlist = util::new_tempfile_name()+L".src";
        TRACE(3,L"Creating phonetics list in "+phonlist);
        util::open_utf8_file(fphon,phonlist);
        if (fphon.fail()) ERROR_CRASH(L"Error creating temp file "+phonlist);      
      }

      // open given dictionary
      wifstream fdic;
      util::open_utf8_file(fdic,dic_file);
      if (fdic.fail()) ERROR_CRASH(L"Error opening file "+dic_file);  

      wstring line, key, form, sound;
      while (getline(fdic,line)) {
        wistringstream sin; sin.str(line); 
        // get sound
        sin>>key;

        if (dic_type==PHONETIC) 
          // dictionary is already phonetic, just load forms for read sound key
          while (sin>>form) 
            orthography.insert(make_pair(key,form));

        else if (dic_type==ORTHOGRAPHIC) {
          // dictionary is orthographic. Encode key, and store pair sound-key
          sound = ph->get_sound(key);  // compute sound
          fphon<<sound<<endl;           // write sound to temp file
          orthography.insert(make_pair(sound,key));
        }
      }      
      fdic.close();

      // If we encoded an ortohgraphic dictionary, use phonetic version to build the FSM
      if (dic_type==ORTHOGRAPHIC) {
        fphon.close();
        dic_file=phonlist;
      }
    }

    // create FSM 
    TRACE(3,L"Creating foma FSMs ");
    sed = new foma_FSM(dic_file,cost_file);
    sed->set_cutoff_threshold(DistanceThreshold);

    // remove temp 
    if (DistanceType==PHONETIC and dic_type==ORTHOGRAPHIC) 
      remove(util::wstring2string(phonlist).c_str());

    TRACE(1,L"alternatives succesfully created");
  }



  ////////////////////////////////////////////////
  /// Destroy alternatives module, close database, delete objects.
  ////////////////////////////////////////////////

  alternatives::~alternatives(){
    // delete phonetics module
    delete ph;
    // delete Foma FSA
    delete sed;
  }


  ////////////////////////////////////////////////
  /// filter given candidate and decide if it is a valid alternative.
  ////////////////////////////////////////////////

  void alternatives::filter_candidate(const wstring &form, const wstring &candidate, int distance, map<wstring,int> & filtered) const {

    // If the alternate form is different than the actual, add it.
    // (it might be equal if we searched for a known word)
    TRACE(3,L"     Filtering candidate "+candidate);
    if (candidate!=form && labs(form.size()-candidate.size())<=MaxSizeDiff) {
      map<wstring,int>::iterator p=filtered.find(candidate);
      if (p==filtered.end()) {  
        // if candidate is new, add it
        filtered.insert(make_pair(candidate,distance));
        TRACE(3,L"     - added alternative ("+candidate+L","+util::int2wstring(distance)+L")"); 
      }
      else {
        // if candidate is not new, keep minimum distance
        p->second = min(p->second,distance);
        TRACE(3,L"     - updated alternative distance ("+candidate+L","+util::int2wstring(p->second)+L")"); 
      }
    }
  }


  ////////////////////////////////////////////////////////////////////////
  /// adds the new words that are valid alternatives.
  ////////////////////////////////////////////////////////////////////////

  void alternatives::filter_alternatives(const list<pair<wstring,int> > &alts, word &w) const {

    // start with existing alternatives (if any)
    map<wstring,int> filtered;
    for (list<pair<wstring,int> >::const_iterator a=w.alternatives_begin(); a!=w.alternatives_end(); a++) 
      filtered.insert(*a);

    if (DistanceType==ORTHOGRAPHIC) {
      TRACE(3,L"   Selecting orthographic alternatives ");
      for (list<pair<wstring,int> >::const_iterator a=alts.begin(); a!=alts.end(); a++) 
        filter_candidate(w.get_lc_form(), a->first, a->second, filtered);
    }

    else if (DistanceType==PHONETIC) {
      // if there is an exact phonetic match, lower the threshold 
      // for filtering (i.e. be pickier for non-exact matches)
      int max = DistanceThreshold;
      if (not alts.empty() and alts.begin()->second==0) max = max/2;
    
      // select alternatives suggested by PHONETIC distance
      TRACE(3,L"   Selecting phonetic alternatives ");
      for (list<pair<wstring,int> >::const_iterator a=alts.begin(); a!=alts.end() && a->second<=max; a++) {
        // recover all dictionary words that correspond to that sound
        TRACE(3,L"    Check candidate orthographic forms for alternative "+a->first);
        typedef multimap<wstring,wstring>::const_iterator mm_it;
        pair<mm_it,mm_it> m = orthography.equal_range(a->first);
        for (mm_it p=m.first; p!=m.second; p++) 
          filter_candidate(w.get_lc_form(), p->second, a->second, filtered);
      }
    }

    w.clear_alternatives();
    // for each selected alternative, add it to the word
    for (map<wstring,int>::const_iterator f=filtered.begin(); f!=filtered.end(); f++) 
      // add new pair form-distance to final alternatives list.
      w.get_alternatives().push_back(*f);

    w.get_alternatives().sort(util::ascending_second<wstring,int>);
  }


  ////////////////////////////////////////////////////////////////////////
  /// Navigates the sentence adding alternative words (possible correct spelling data)
  ////////////////////////////////////////////////////////////////////////

  void alternatives::analyze(sentence &se) const {

    for (sentence::iterator pos=se.begin(); pos!=se.end(); ++pos)  {    

      bool check = false;
      if (pos->get_n_analysis()) { // Known word, check if some tag matches CheckKnownTags
        for (word::iterator a=pos->begin(); a!=pos->end() and not check; a++) 
          check = CheckKnownTags.search(a->get_tag());
      }
      else 
        check = CheckUnknown;  // unknown word. Check it if config file requires it 
                      
      TRACE(3,L"Checking word "+pos->get_form()+L": "+(check?L"yes":L"no"));

      if (check) {

        list<pair<wstring,int> > alts;
        if (DistanceType==ORTHOGRAPHIC) {
          // get EDIT alternatives
          TRACE(3,L"get ORTHO alternatives for "+pos->get_lc_form());
          sed->get_similar_words(pos->get_lc_form(), alts);
          TRACE(4,L" found "+util::int2wstring(alts.size())+L" ORTHO alternatives");
        }

        else if (DistanceType==PHONETIC) {
          // compute phonetics for this word
          wstring phon = ph->get_sound(pos->get_lc_form());
          TRACE(3,L"get PHON alternatives for "+phon);
          // get PHONETIC alternatives  
          sed->get_similar_words(phon, alts);
          TRACE(4,L" found "+util::int2wstring(alts.size())+L" PHON alternatives");
        }

        // filter and add the obtained words as new analysis
        if (not alts.empty()) filter_alternatives(alts,*pos);
      }
    }  
  }


  ////////////////////////////////////////////////////////////////////////
  /// Provide direct access to results of underlying automata, in case 
  /// caller only want the list of strings
  ////////////////////////////////////////////////////////////////////////

  void alternatives::get_similar_words(const wstring &form, list<pair<wstring,int> > & results) const {

    if (DistanceType==ORTHOGRAPHIC)
      sed->get_similar_words(form, results);

    else if (DistanceType==PHONETIC) {
      // encode given word
      wstring phon = ph->get_sound(form);
      TRACE(4,L" Get words similar to "+form+L".  Looking for entries sounding like "+phon);
      list<pair<wstring,int> > aux;
      set<wstring> seen;

      // get similar sounds
      sed->get_similar_words(phon, aux);

      // recover all dictionary words that correspond to similar sounds  
      for (list<pair<wstring,int> >::const_iterator a=aux.begin(); a!=aux.end(); a++) {
        TRACE(5,L"   Found similar sound "+a->first);
        typedef multimap<wstring,wstring>::const_iterator mm_it;
        pair<mm_it,mm_it> m = orthography.equal_range(a->first);
        for (mm_it p=m.first; p!=m.second; p++) {
          TRACE(5,L"      ... from word "+p->second);
          // select word if it is not a trivial match, it is not already in the list, and
          // lenght differenece is in limits.
          if (labs(form.size()-p->second.size())<=MaxSizeDiff && seen.find(p->second)==seen.end()) 
            results.push_back(make_pair(p->second,a->second));
        }
      }
    }

  }


} // namespace

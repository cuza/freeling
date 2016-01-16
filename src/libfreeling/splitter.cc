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

#include "freeling/morfo/splitter.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"SPLITTER"
#define MOD_TRACECODE SPLIT_TRACE

#define SAME 100
#define VERY_LONG 1000

  ///////////////////////////////////////////////////////////////
  /// Create a sentence splitter.
  ///////////////////////////////////////////////////////////////

  splitter::splitter(const wstring &splitFile) { 

    enum sections {GENERAL, MARKERS, SENT_END, SENT_START};
    config_file cfg;
    cfg.add_section(L"General",GENERAL);
    cfg.add_section(L"Markers",MARKERS);
    cfg.add_section(L"SentenceEnd",SENT_END);
    cfg.add_section(L"SentenceStart",SENT_START);

    if (not cfg.open(splitFile))
      ERROR_CRASH(L"Error opening file "+splitFile);

    // default values and initializations   
    SPLIT_AllowBetweenMarkers = true;
    SPLIT_MaxWords = 0;
    betweenMrk=false;
    no_split_count=0; 
    mark_type.clear();
    mark_form.clear();

    // process each content line according to the section where it is found
    int nmk=1;
    wstring line;
    while (cfg.get_content_line(line)) {

      wistringstream sin;
      sin.str(line);

      switch (cfg.get_section()) {

      case GENERAL: {
        // reading general options
        wstring name;
        sin>>name;
        if (name==L"AllowBetweenMarkers") sin>>SPLIT_AllowBetweenMarkers;
        else if (name==L"MaxWords") sin>>SPLIT_MaxWords;
        else ERROR_CRASH(L"Unexpected splitter option '"+name+L"'");
        break;
      }

      case MARKERS: {
        wstring open,close;
        // reading open-close marker pairs (parenthesis, etc)
        sin>>open>>close;
        if (open!=close) {
          markers.insert(make_pair(open,nmk));   // positive for open, negative for close
          markers.insert(make_pair(close,-nmk));
        }
        else {  // open and close are the same (e.g. double "quotes")
          markers.insert(make_pair(open, SAME+nmk));   // both share the same code, but its distinguishable
          markers.insert(make_pair(close, SAME+nmk));
        }
        nmk++;
        break;
      }

      case SENT_END: {
        // reading end-sentence delimiters
        wstring name;
        bool value;
        sin>>name>>value;
        enders.insert(make_pair(name,value));
        break;
      }

      case SENT_START: {
        // reading start-sentence delimiters
        starters.insert(line);
        break;
      }

      default: break;
      }
    }

    cfg.close(); 

    TRACE(1,L"analyzer succesfully created");
  }


  ///////////////////////////////////////////////////////////////
  ///  Accumulate the word list v into the internal buffer.
  ///  If a sentence marker is reached (or flush flag is set), 
  ///  return all sentences currently in buffer, and clean buffer.
  ///  If a new sentence is started but not completed, keep
  ///  in buffer, and wait for further calls with more data.
  ///////////////////////////////////////////////////////////////

  void splitter::split(const list<word> &v, bool flush, list<sentence> &ls) {
    list<word>::const_iterator w;
    map<wstring,bool>::const_iterator e;
    map<wstring,int>::const_iterator m;
    bool check_split;

    TRACE(3,L"Looking for a sentence marker. Max no split is: "+util::int2wstring(SPLIT_MaxWords));
    TRACE_WORD_LIST(4,v);

    // clear list of sentences from previous use
    ls.clear();

    for (w=v.begin(); w!=v.end(); w++) {
    
      // check whether we found a marker (open/close parenthesis, quote, etc)
      m=markers.find(w->get_form());

      check_split=true;

      // last opened marker is being closed, pop from stack, if
      // stack is empty, we are no longer inbetween markers
      if (betweenMrk and !SPLIT_AllowBetweenMarkers and 
          m!=markers.end() and m->second==(m->second>SAME?1:-1)*mark_type.front())
        {
          TRACE(3,L"End no split period. marker="+m->first+L" code:"+util::int2wstring(m->second));
          mark_type.pop_front(); 
          mark_form.pop_front(); 
          if (mark_type.empty()) {
            betweenMrk=false;
            no_split_count=0; 
          }
          else
            no_split_count++;

          buffer.push_back(*w);
          check_split=false;
        }

      // new marker being opened, push onto open markers stack, 
      // we enter inbetween markers (or stay if we already were)
      else if (m!=markers.end() and m->second>0 and !SPLIT_AllowBetweenMarkers) 
        {
          mark_form.push_front(m->first);  
          mark_type.push_front(m->second); 
          TRACE(3,L"Start no split period, marker "+m->first+L" code:"+util::int2wstring(m->second));
          betweenMrk=true;
          no_split_count++;
          buffer.push_back(*w);
          check_split=false;
        }

      // regular word, inside markers. Do not split unless AllowBetweenMarkers is set.
      // Warn if we have been inside markers without splitting for very long.
      else if (betweenMrk) {

        TRACE(3,L"no-split flag continues set. word="+w->get_form()+L" expecting code:"+util::int2wstring(mark_type.front())+L" (closing "+mark_form.front()+L")");      
        no_split_count++;

        if (SPLIT_MaxWords==0 or no_split_count<=SPLIT_MaxWords) {
          check_split=false;
          buffer.push_back(*w);
        }

        if (no_split_count==VERY_LONG)  {
          WARNING(L"Ridiculously long sentence between markers at token '"+w->get_form()+L"' at input offset "+util::int2wstring(w->get_span_start())+L".");
          if (no_split_count==VERY_LONG+5) {
            WARNING(L"...etc...");
            WARNING(L"Expecting code "+util::int2wstring(mark_type.front())+L" (closing "+mark_form.front()+L") for over "+util::int2wstring(VERY_LONG)+L" words. Probable marker mismatch in document.");
            WARNING(L"If this causes crashes try setting AllowBetweenMarkers=1 or");
            WARNING(L"setting a value other than 0 for MaxWords in your splitter");
            WARNING(L"configuration file (e.g. splitter.dat)");
          }
        }
      }

      // regular word outside markers, or AllowBtwenMarkers=1, 
      // or MaxWords exceeded.  -->  Check for a sentence ending.
      if (check_split) {      
        // check for possible sentence ending. 
        e = enders.find(w->get_form());

        if (e!=enders.end()) {
          // We split if the delimiter is "sure" (e->second==true) or if 
          // context checking for sentence end returns true. 
          if (e->second || end_of_sentence(w,v)) {
            TRACE(2,L"Sentence marker ["+w->get_form()+L"] found");
            // Complete the sentence
            buffer.push_back(*w);
            // store it in the results list
            ls.push_back(buffer);

            // Clear sentence, look for a new one
            buffer.clear();
            // reset state
            betweenMrk=false; 
            no_split_count=0; 
            mark_type.clear(); mark_form.clear(); 
          }
          else {
            // context indicated it was not a sentence ending.
            TRACE(3,w->get_form()+L" is not a sentence marker here");
            buffer.push_back(*w);
          }
        }
        else {
          // Normal word. Accumulate to the buffer.
          TRACE(3,w->get_form()+L" is not a sentence marker here");
          buffer.push_back(*w);
        }
      }
    }

    // if flush is set, do not retain anything
    if (flush && !buffer.empty()) { 
      TRACE(3,L"Flushing the remaining words into a sentence");
      // add sentence to return list
      ls.push_back(buffer);         
      // reset state
      buffer.clear(); no_split_count=0; 
      mark_type.clear(); mark_form.clear(); 
      betweenMrk=false; 
    }

    TRACE_SENTENCE_LIST(1,ls);
  }


  ///////////////////////////////////////////////////////////////
  ///  Split and return a copy of the sentences
  ///////////////////////////////////////////////////////////////

  list<sentence> splitter::split(const list<word> &v, bool flush) {

    list<sentence> ls;
    split (v, flush, ls);
    return ls;
  }


  ///////////////////////////////////////////////////////////////
  ///  Check whether a word is a sentence end (eg a dot followed
  /// by a capitalized word).
  ///////////////////////////////////////////////////////////////

  bool splitter::end_of_sentence (list<word>::const_iterator w, const list<word> &v) const {
    list<word>::const_iterator r;
    wstring f,g;

    // Search at given position w for the next word, to decide whether w contains a marker.

    if (w==--v.end()) {
      // Last word in the list. We don't know whether we can consider a sentence end. 
      // Probably we'd better wait for next line, but we don't in this version.
      return true;
    }
    else {
      // Not last word, check the following word to see whether it is uppercase
      r=w; r++;
      f=r->get_form();

      // See if next word a typical sentence starter or its first letter is uppercase
      return (util::RE_is_capitalized.search(f) || starters.find(f)!=starters.end());
    }
  }


} // namespace

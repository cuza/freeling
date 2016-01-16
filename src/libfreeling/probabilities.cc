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
#include <set>

#include "freeling/morfo/configfile.h"
#include "freeling/morfo/probabilities.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"PROBABILITIES"
#define MOD_TRACECODE PROB_TRACE


  ///////////////////////////////////////////////////////////////
  /// Create a probability assignation module, loading
  /// appropriate file.
  ///////////////////////////////////////////////////////////////

  probabilities::probabilities(const std::wstring &probFile, double Threshold) : RE_PunctNum(RE_FZ) {
    wstring line;
    wstring key,clas,frq,tag,ftags;
    map<wstring,double>::iterator k;
    map<wstring,double> temp_map;
    double probab, sumUnk, sumSing, count;

    activate_guesser=true;

    ProbabilityThreshold = Threshold;

    // default values, in case config file doesn't specify them.
    BiassSuffixes=0.3;
    LidstoneLambda=0.1;

    enum sections {SINGLE_TAG,CLASS_TAG,FORM_TAG,UNKNOWN,
                   THEETA,SUFFIXES,SUFF_BIASS,LAMBDA,TAGSET};
    config_file cfg;  
    cfg.add_section(L"SingleTagFreq",SINGLE_TAG);
    cfg.add_section(L"ClassTagFreq",CLASS_TAG);
    cfg.add_section(L"FormTagFreq",FORM_TAG);
    cfg.add_section(L"UnknownTags",UNKNOWN);
    cfg.add_section(L"Theeta",THEETA);
    cfg.add_section(L"Suffixes",SUFFIXES);
    cfg.add_section(L"BiassSuffixes",SUFF_BIASS);
    cfg.add_section(L"LidstoneLambda",LAMBDA);
    cfg.add_section(L"TagsetFile",TAGSET);

    if (not cfg.open(probFile))
      ERROR_CRASH(L"Error opening file "+probFile);

    sumUnk=0; sumSing=0; long_suff=0;
    while (cfg.get_content_line(line)) {
    
      wistringstream sin;
      sin.str(line);

      switch (cfg.get_section()) {

      case SINGLE_TAG: {
        // reading Single tag frequencies
        sin>>key>>frq;
        probab=util::wstring2double(frq);
        single_tags.insert(make_pair(key,probab));
        sumSing += probab;
        break;
      }
        
      case CLASS_TAG: {
        // reading tag class frequencies
        temp_map.clear();
        sin>>key;
        while (sin>>tag>>frq) {
          probab=util::wstring2double(frq);
          temp_map.insert(make_pair(tag,probab));
        }
        class_tags.insert(make_pair(key,temp_map));
        break;
      }

      case FORM_TAG: {
        // reading form tag frequencies
        temp_map.clear();
        sin>>key>>clas;
        while (sin>>tag>>frq) {
          probab=util::wstring2double(frq);
          temp_map.insert(make_pair(tag,probab));
        }
        lexical_tags.insert(make_pair(key,temp_map));
        break;
      }

      case UNKNOWN: {
        // reading tags for unknown words
        sin>>tag>>frq;
        probab=util::wstring2double(frq);
        sumUnk += probab;
        unk_tags.insert(make_pair(tag,probab));
        break;
      }

      case THEETA: {
        // reading theeta parameter
        sin>>frq;
        theeta=util::wstring2double(frq);
        break;
      }

      case SUFFIXES: {
        // reading suffixes for unknown words
        temp_map.clear();
        sin>>key>>frq;
        long_suff = (key.length()>long_suff ? key.length() : long_suff);
        count = util::wstring2double(frq);
        while (sin>>tag>>frq) {
          probab=util::wstring2double(frq)/count;
          temp_map.insert(make_pair(tag,probab));
        }
        unk_suffs.insert(make_pair(key,temp_map));
        break;
      } 

      case SUFF_BIASS: {
        // reading BiassSuffixes parameter
        sin>>frq;
        BiassSuffixes=util::wstring2double(frq);
        break;
      }

      case LAMBDA: {
        // reading LidstoneLambda parameter
        sin>>frq;
        LidstoneLambda=util::wstring2double(frq);
        break;
      }

      case TAGSET: {
        // reading Tagset description file name
        sin>>ftags;
        break;
      }

      default: break;
      }
    }
    cfg.close(); 
    
    // normalizing the probabilities in unk_tags map
    for (k=unk_tags.begin(); k!=unk_tags.end(); k++)
      k->second = k->second / sumUnk;

    for (k=single_tags.begin(); k!=single_tags.end(); k++) 
      k->second = k->second / sumSing;

    // load tagset description
    wstring path=probFile.substr(0,probFile.find_last_of(L"/\\")+1);
    Tags = new tagset(util::absolute(ftags,path));

    TRACE(3,L"analyzer succesfully created");
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Destructor
  /////////////////////////////////////////////////////////////////////////////

  probabilities::~probabilities()  {
    delete Tags;
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Annotate probabilities for each analysis of given word
  /////////////////////////////////////////////////////////////////////////////

  void probabilities::annotate_word(word &w) const {
  
    double sum;
    int na=w.get_n_analysis();
    TRACE(2,L"--Assigning probabilitites to: "+w.get_form());
  
    // word found in dictionary, punctuation mark, number, or with retokenizable analysis 
    //  and with some analysis
    if ( na>0 && (w.found_in_dict() || w.find_tag_match(RE_PunctNum) || w.has_retokenizable())) {
      TRACE(2,L"Form with analysis. Found in dict ("+wstring(w.found_in_dict()?L"yes":L"no")+L") or punctuation ("+wstring(w.find_tag_match(RE_PunctNum)?L"yes":L"no")+L") or has_retok ("+wstring(w.has_retokenizable()?L"yes":L"no")+L")");

      // smooth probabilities for original analysis
      smoothing(w);
      sum=1;
    }
    // word not found in dictionary, may (or may not) have
    // tags set by other modules (NE, dates, suffixes...)
    else  if (activate_guesser) {
      // form is unknown in the dictionary
      TRACE(2,L"Form with NO analysis. Guessing");
    
      // set uniform distribution for analysis from previous modules.
      const double mass=1.0;
      for (word::iterator li=w.begin(); li!=w.end(); li++)
        li->set_prob(mass/w.get_n_analysis());
    
      // guess possible tags, keeping some mass for previously assigned tags.
      // setting mass to higher values above, will give more weight to 
      // existing tags.
      sum=guesser(w,mass);
    
      // normalize probabilities of all accumulated tags
      for (word::iterator li=w.begin();  li!=w.end(); li++)
        li->set_prob(li->get_prob()/sum);
    
      // get number of analysis again, in case the guesser added some.
      na=w.get_n_analysis();
    }
  
    w.sort(std::greater<analysis>()); // sort analysis by decreasing probability,

    //sorting may scramble selected/unselected list, and the tagger will
    // need all analysis to be selected. This should be transparent here
    // (probably move it to class 'word').
    w.select_all_analysis();

    // if any of the analisys of the word was retokenizable, assign probabilities
    // to its sub-words, just in case they are selected.
    for (word::iterator an=w.begin();  an!=w.end(); an++) {
      list<word> & rtk=an->get_retokenizable();
      for (list<word>::iterator k=rtk.begin(); k!=rtk.end(); k++) 
        annotate_word(*k);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Annotate probabilities for each analysis of each word in given sentence,
  /// using given options.
  /////////////////////////////////////////////////////////////////////////////

  void probabilities::analyze(sentence &se) const {
    sentence::iterator i;
    for (i=se.begin(); i!=se.end(); i++)
      annotate_word(*i);    

    TRACE_SENTENCE(1,se);
  }


  /////////////////////////////////////////////////////////////////////////////
  /// Smooth probabilities for the analysis of given word
  /////////////////////////////////////////////////////////////////////////////

  void probabilities::smoothing(word &w) const {

    int na=w.get_n_analysis();
    if (na==1) {
      // form is inambiguous
      TRACE(2,L"Inambiguous form, set prob to 1");
      w.begin()->set_prob(1);
      return; // we're done here
    }

    // form has analysis. begin probability back-off
    TRACE(2,L"Form with analysis. Smoothing probabilites.");

    // count occurrences of short tags 
    map<wstring,double> tags_short;
    for (word::iterator li=w.begin(); li!=w.end(); li++) {
      std::pair<std::map<std::wstring, double>::iterator, bool> inserted;
      inserted = tags_short.insert(make_pair(Tags->get_short_tag(li->get_tag()), 1.0));
      if (not inserted.second) inserted.first->second++;     
    }
  
    map<wstring,map<wstring,double> >::const_iterator it;
    const map<wstring,double> * temp_map=NULL;
    map<wstring,double>::const_iterator p;
    wstring form=w.get_lc_form();
    double sum=0;
    bool using_backoff;

    it = lexical_tags.find(form);
    if (it!=lexical_tags.end()) {
      // word found in lexical probabilities list. Use them straightforwardly.
      TRACE(2,L"Form '"+form+L"' lexical probabilities found");
      temp_map= &(it->second);    
      using_backoff=false;
    }  
    else {
      // No lexical statistics for this word. Back-off to ambiguity class
      TRACE(2,L"Form '"+form+L"' lexical probabilities not found.");
      using_backoff=true;

      // build word ambiguity class with and without NP tags
      wstring c=L""; wstring cNP=L"";
      for (map<wstring,double>::iterator x=tags_short.begin(); x!=tags_short.end(); x++) {
        cNP += L"-"+x->first;
        if ( x->first != L"NP" ) c += L"-"+x->first;
      }
      cNP=cNP.substr(1);   
      if (c.empty())
        WARNING(L"Empty ambiguity class for word '"+w.get_form()+L"'. Duplicate NP analysis??");
      else 
        c=c.substr(1);
      TRACE(3,L"Ambiguity class: ["+cNP+L"].   Secondary class: ["+c+L"].");

      it = class_tags.find(cNP);
      if (it!=class_tags.end()) {
        TRACE(2,L"Ambiguity class '"+cNP+L"' probabilities found.");
        temp_map= &(it->second);          
      }
      else if (c!=cNP) { // we may find the class without NP (secondary class)
        it = class_tags.find(c);
        if (it!=class_tags.end()) {
          TRACE(2,L"Secondary ambiguity class '"+c+L"' probabilities found.");
          temp_map= &(it->second);          
        }
      }

      // No statistics found for any ambiguity class. Back-off to unigram probabilities.
      if (temp_map==NULL) temp_map=&single_tags;
    }

    // Compute total number of observations
    for (map<wstring,double>::iterator x=tags_short.begin(); x!=tags_short.end(); x++) {
      // find tag in the selected map (lexical entry, class, or unigram)
      p = temp_map->find(x->first);
      // It should be there, but it may not be due to inconsistencies in training corpus.
      // If it is not found, just use a count of zero, and let the smooth take care of it
      sum += (p==temp_map->end() ? 0 : p->second) * x->second;
    }
  
    if (not using_backoff) {
      // For observed lexical probabilities, smooth probabilities with Lidstone's Law
      double norm = sum + LidstoneLambda*na;
      for (word::iterator li=w.begin(); li!=w.end(); li++) {
        p = temp_map->find(Tags->get_short_tag(li->get_tag()));
        li->set_prob(((p==temp_map->end() ? 0 : p->second) + LidstoneLambda)/norm);
      }
    }
    else {
      // For back-off probabilities, smooth probabilities with Laplace's Law 
      // since we have higher counts.
      double norm = sum + na;
      for (word::iterator li=w.begin(); li!=w.end(); li++) {
        p = temp_map->find(Tags->get_short_tag(li->get_tag()));
        li->set_prob(((p==temp_map->end() ? 0 : p->second)+1)/norm);
      }
    }

    if (using_backoff) {
      /// if using backoff, combine with suffix information to get better estimation
      TRACE(2,L"Using suffixes to smooth probs.");
      double norm=0;
      double* p = new double[w.size()];
      int i=0;
      for (word::iterator li=w.begin(); li!=w.end(); li++) {
        // suffix-based prob
        p[i] = compute_probability(li->get_tag(), li->get_prob(), w.get_form());
        norm += p[i];
        i++;
      }
    
      // interpolation. BiassSuffixes = weight for suffix contribution
      i=0;
      for (word::iterator li=w.begin(); li!=w.end(); li++) {
        li->set_prob((1-BiassSuffixes)*li->get_prob() + BiassSuffixes*p[i]/norm);
        i++;
      }

      delete [] p;
    }
  }



  /////////////////////////////////////////////////////////////////////////////
  /// Compute probability of a tag given a word suffix.
  /////////////////////////////////////////////////////////////////////////////

  double probabilities::compute_probability(const std::wstring &tag, double prob, const std::wstring &s) const {
    map<wstring,map<wstring,double> >::const_iterator is;
    map<wstring,double>::const_iterator it;
    double x,pt;

    x=prob;  
    int spos=s.size();
    bool found=true;

    TRACE(4,L" suffixes. Tag "+tag+L" initial prob="+util::double2wstring(x));
    while (spos>0 && found) {
      spos--;
      // get tag probability list for current suffix
      is = unk_suffs.find(s.substr(spos));
      found = (is != unk_suffs.end());
      if (found) {
        // search tag in suffix probability list. 
        it = (is->second).find(tag);
        if (it != (is->second).end()) {
          pt = it->second;
          TRACE(4,L"       found prob for sufix -"+s.substr(spos));
        }
        else {
          pt = 0;
          TRACE(4,L"       NO prob found for sufix -"+s.substr(spos));
        }
        x = (pt+theeta*x)/(1+theeta);
      }
    }
    TRACE(4,L"             final prob="+util::double2wstring(x));
    return x;
  }


  /////////////////////////////////////////////////////////////////////////////
  /// Guess possible tags, keeping some mass for previously assigned tags    
  /////////////////////////////////////////////////////////////////////////////

  double probabilities::guesser(word &w, double mass) const {

    wstring form=w.get_lc_form();
  
    // mass = mass assigned so far.  This gives some more probability to the 
    // preassigned tags than to those computed from suffixes.
    double sum= (w.get_n_analysis()>0 ? mass : 0);
    double sum2=0;
    
    TRACE(2,L"initial sum=: "+util::double2wstring(sum));
  
    // remeber initial tags (if any)
    set<wstring> stags;
    for (word::iterator li=w.begin(); li!=w.end(); li++)
      stags.insert(Tags->get_short_tag(li->get_tag()));

    // to store analysis under threshold, just in case
    list<analysis> la;
    // for each possible tag, compute probability
    analysis a;
    for (map<wstring,double>::const_iterator t=unk_tags.begin(); t!=unk_tags.end(); t++) {

      TRACE(2,L"  guesser: checking tag "+t->first);
      // See if it was already there, set by some other module
      bool hasit = (stags.find(Tags->get_short_tag(t->first))!=stags.end());
    
      // if we don't have it, consider including it in the list
      if (!hasit) {      
        double p = compute_probability(t->first,t->second,form);
        a.init(form,t->first);
        a.set_prob(p);
      
        TRACE(2,L"   tag:"+t->first+L" ("+(hasit?L"had it":L"new")+L")  pr="+util::double2wstring(p)+L" "+util::double2wstring(t->second));
      
        // if the tag is new and higher than the threshold, keep it.
        if (p >= ProbabilityThreshold) {
          sum += p;
          w.add_analysis(a);
          TRACE(2,L"    added. sum is: "+util::double2wstring(sum));
        }
        // store candidates under treshold, just in case we need them later
        else  {
          sum2 += p;
          la.push_back(a);
        }
      }
    }
  
    // if the word had no analysis, and no candidates passed the threshold, assign them
    // anyway, to avoid words with zero analysis
    if (w.get_n_analysis()==0) {    
      w.set_analysis(la);
      sum = sum2;
    }

    return sum;
  }



  /////////////////////////////////////////////////////////////////////////////
  /// Turn guesser on/off.
  /////////////////////////////////////////////////////////////////////////////

  void probabilities::set_activate_guesser(bool b) {
    activate_guesser=b;
  }
} // namespace

//////////////////////////////////////////////////////////////////
//
//    Copyright (C) 2006   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU General Public License
//    (GNU GPL) as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
//    General Public License for more details.
//
//    You should have received a copy of the GNU General Public License 
//    along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: Muntsa Padro (mpadro@lsi.upc.edu)
//             TALP Research Center
//             despatx Omega.S107 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////
 
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

#include "freeling/morfo/configfile.h"
#include "freeling/morfo/idioma.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"IDIOMA"
#define MOD_TRACECODE LANGIDENT_TRACE

  // /////////////////////// Class idioma //////////////////////////
  // 
  //   Class that implements a visible Markov's that calculates 
  //   the probability that a text is in a certain language.
  // 
  // ///////////////////////////////////////////////////////////////


  /////////////////////////////////////
  /// Empty constructor
  /////////////////////////////////////

  idioma::idioma() {}

  ///////////////////////////////////////////////////////////
  /// Create and instance of the VMM with the given language n-gram model.
  ////////////////////////////////////////////////////////////

  idioma::idioma(const wstring &modelFile) {

    map<wstring,double> tgA,bgA,ugA;
    map<pair<wstring,wstring>,double> tgB,bgB,ugB;
    double totfqPi=0, totfqA=0;

    // scale factor
    scale=0;

    // config file
    enum sections {CODE,SCALE,TRANSITION,UNIGRAM,INITIAL};
    config_file cfg;  
    cfg.add_section(L"CODE",CODE);
    cfg.add_section(L"SCALE",SCALE);
    cfg.add_section(L"TRANSITION",TRANSITION);
    cfg.add_section(L"UNIGRAM",UNIGRAM);
    cfg.add_section(L"INITIAL",INITIAL);

    if (not cfg.open(modelFile))
      ERROR_CRASH(L"Error opening file "+modelFile);
  
    // load list of languages and their model files
    wstring line; 
    while (cfg.get_content_line(line)) {

      switch (cfg.get_section()) {

      case CODE: {// read language code
        LangCode=line;
        break;
      }

      case SCALE: {// read scale factor
        wistringstream sin;
        sin.str(line);
        sin>>scale;
        break;
      }

      case TRANSITION: {  // read transition freqs
        wistringstream sin;
        sin.str(line);
        wstring t1,t2;
        double fq;
        sin>>t1>>t2>>fq;      // read 1 n-gram, 1 next char, and a transition freq
        t1=from_writable(t1); // convert writable trigrams to actual chars.
        t2=from_writable(t2); 

        switch (t1.size()) {
        case 3:  // accumulate trigram counts
          increment(tgA,t1,fq);
          increment(tgB,t1,t2,fq);
          break;
        case 2:  // accumulate bigram counts
          increment(bgA,t1,fq);  
          increment(bgB,t1,t2,fq);
          break;
        case 1:  // accumulate unigram counts
          increment(ugA,t1,fq);  
          increment(ugB,t1,t2,fq);
          break;
        default:
          break;
        }

        break;
      }

      case UNIGRAM: { // read unigram freqs
        wistringstream sin;
        sin.str(line);
        wstring t1;
        double fq;
        sin>>t1>>fq;      // read 1 unigram and its freq
        t1=from_writable(t1); // convert writable trigrams to actual chars.
        pa_nom.insert(make_pair(t1,fq));  // store frequency;
        totfqA += fq;       // total count
        break;
      }

      case INITIAL: { // read initial freqs
        wistringstream sin;
        sin.str(line);
        wstring t1;
        double fq;
        sin>>t1>>fq; // read 1 trigrams and initial freq.
        t1=from_writable(t1);
        ppi_nom.insert(make_pair(t1,fq));
        // accumulate total of observations
        totfqPi += fq;
        break;
      }

      default: break;
      }
    }
    cfg.close();

    // convert init state counts to probabilities
    map<wstring,double>::iterator k;
    for (k=ppi_nom.begin(); k!=ppi_nom.end(); k++) 
      k->second = log(k->second/totfqPi);

    // convert unigram counts to probabiltites
    for (k=pa_nom.begin(); k!=pa_nom.end(); k++) 
      k->second = log(k->second/totfqA);

    // convert trigram transition counts to 4-gram probabilities
    map<pair<wstring,wstring>,double>::const_iterator i;
    for (i=tgB.begin(); i!=tgB.end(); i++) {
      map<wstring,double>::const_iterator a=tgA.find(i->first.first);
      double p = log(i->second/a->second);
      pa_nom.insert(make_pair(i->first.first+i->first.second,p)); 
    }

    // convert bigram transition counts to 3-gram probabilities
    for (i=bgB.begin(); i!=bgB.end(); i++) {
      map<wstring,double>::const_iterator a=bgA.find(i->first.first);
      double p = log(i->second/a->second);
      pa_nom.insert(make_pair(i->first.first+i->first.second,p)); 
    }

    // convert unigram transition counts to 2-gram probabilities
    for (i=ugB.begin(); i!=ugB.end(); i++) {
      map<wstring,double>::const_iterator a=ugA.find(i->first.first);
      double p = log(i->second/a->second);
      pa_nom.insert(make_pair(i->first.first+i->first.second,p)); 
    }

    TRACE(2,L"Loaded model for language "+LangCode);
  }

  //////////////////////////////////////////////////////////
  /// return code for current language
  //////////////////////////////////////////////////////////

  wstring idioma::get_language_code() const {
    return LangCode;
  }

  //////////////////////////////////////////////////////////
  /// Compute probabiltiy for given sequence according to 
  /// current model. Parameter "len" gets the actual length 
  /// of the sequence used after skipping redundant whitespaces.
  //////////////////////////////////////////////////////////

  double idioma::sequence_probability(wistream &f, size_t &len) const {

    wchar_t c1,c2,c3,c4;
    double prob=0;

    TRACE(3,L"Computing sequence probability.");

    // initial trigram
    initial_trigram(f,c1,c2,c3);
    // initialize sequence probability
    prob = ProbPi(trigram(c1,c2,c3));
    TRACE(4,L"Initial state. prob="+util::double2wstring(prob));

    len=1;
    f>>noskipws>>c4; 
    c4=towlower(c4);
    while (not f.eof()) {

      TRACE(3,L"Current trigram: ("+to_writable(trigram(c1,c2,c3))+L")  next: ("+to_writable(trigram(c2,c3,c4))+L")");

      /// Update sequence probability
      prob += ProbA(trigram(c1,c2,c3),c4);
      TRACE(4,L"    Added transition. accumulated prob="+util::double2wstring(prob));

      if (c4==L'\n') {
        // Found end-of-paragraph. 
        // Reset to init state for next paragraf
        initial_trigram(f,c1,c2,c3);   
        if (!f.eof()) {
          prob += ProbPi(trigram(c1,c2,c3));
          f>>noskipws>>c4;        
          c4=towlower(c4);
        }
        TRACE(4,L"End of paragraph. Current trigram: ("+to_writable(trigram(c1,c2,c3))+L")  next: ("+to_writable(trigram(c2,c3,c4))+L")");
        TRACE(4,L"    Added Pi. accumulated prob="+util::double2wstring(prob));
      }
      else { 
        // normal next, shift trigram window.
        c1=c2; c2=c3; c3=c4; 
        f>>noskipws>>c4;
        if (c3==L' ' || c3==L'\t') {
          while (!f.eof() && (c4==L' ' || c4==L'\t')) 
            f>>noskipws>>c4;
          TRACE(4,L"End of word. Skip whitespaces.");
        }
        c4=towlower(c4);
      }
      len++;
    }

    return prob;
  }


  //////////////////////////////////////////////////////////
  /// Compute language probability, normalizing sequence
  /// probability by the senquence length.
  /////////////////////////////////////////////////////////

  double idioma::compute_probability(const wstring &text, double sc) const {

    size_t len;
    wistringstream sin(text);
    double prob= sequence_probability((wistream &)sin, len);

    TRACE(4,L"   Sequence probability="+util::double2wstring(prob)+L"  len="+util::double2wstring(len));

    if (scale!=0) sc=scale;  // if model has local scale value, ignore parameter.
    prob=exp(prob/len)*sc;
    if (prob>1.0) prob=1.0;

    TRACE(4,L"   Normalized & scaled (x"+util::double2wstring(sc)+L") prob="+util::double2wstring(prob));
    return prob;
  }


  //////////////////////////////////////////////////////////
  /// Use given text stream to create a Markov model for
  /// the language. Store model in given filename.
  //////////////////////////////////////////////////////////

  void idioma::train(wistream &f, const wstring &modelFile, const wstring &code) {

    // use new code.
    LangCode=code;
    // clear temporary tables
    pi.clear(); tB.clear(); bB.clear(); uB.clear(); A.clear();
    // train model
    create_model(f);
    // save results
    save_model(modelFile);
    // clear temporary tables.
    pi.clear(); tB.clear(); bB.clear(); uB.clear(); A.clear();
  }

  //////////////////////////////////////////////////////////
  /// Use given text file to create a Markov model for
  /// the language. Store model in given filename.
  //////////////////////////////////////////////////////////

  void idioma::train(const wstring &textFile, const wstring &modelFile, const wstring &code) {
    // open text file
    wifstream f;
    util::open_utf8_file(f, textFile);
    if (f.fail()) ERROR_CRASH(L"Error opening file "+textFile);
    // train from file stream
    train(f,modelFile,code);
  }
  
  // ------------ Private functions of "idioma" -------------------

  /////////////////////////////////////////////////////////////////////
  /// Save current model to given file
  //////////////////////////////////////////////////////////////////////

  void idioma::save_model(const wstring &modelFile) const {
    // open file to save model
    wofstream sout;
    util::open_utf8_file(sout, modelFile);
    if (sout.fail()) ERROR_CRASH(L"Error opening file "+modelFile);

    // Save language code
    sout<<L"<CODE>"<<endl;    
    sout<<LangCode<<endl;
    sout<<L"</CODE>"<<endl;    

    // Save trigram transition table
    sout<<L"<TRANSITION>"<<endl;    
    map<pair<wstring,wstring>,double>::const_iterator i;
    for (i=tB.begin(); i!=tB.end(); i++) {
      wstring k1=to_writable(i->first.first);
      wstring k2=to_writable(i->first.second); 
      sout << k1 <<L" "<< k2 <<L" "<<i->second<<L" "<<endl;
    }
    for (i=bB.begin(); i!=bB.end(); i++) {
      wstring k1=to_writable(i->first.first);
      wstring k2=to_writable(i->first.second); 
      sout << k1 <<L" "<< k2 <<L" "<<i->second<<L" "<<endl;
    }
    for (i=uB.begin(); i!=uB.end(); i++) {
      wstring k1=to_writable(i->first.first);
      wstring k2=to_writable(i->first.second); 
      sout << k1 <<L" "<< k2 <<L" "<<i->second<<L" "<<endl;
    }
    sout<<L"</TRANSITION>"<<endl;
    sout<<L"<UNIGRAM>"<<endl;
    map<wstring,double>::const_iterator a;
    for (a=A.begin(); a!=A.end(); a++) {
      wstring k1=to_writable(a->first);
      sout << k1 <<L" "<<a->second<<L" "<<endl;
    }
    sout<<L"</UNIGRAM>"<<endl;

    // Save initial state probabilities
    sout<<L"<INITIAL>"<<endl;
    map<wstring,double>::const_iterator j;
    for (j=pi.begin(); j!=pi.end(); j++) {
      wstring k=to_writable(j->first); 
      sout << k <<L" "<< j->second<<endl;
    }
    sout<<L"</INITIAL>"<<endl;    

    sout.close();

    TRACE(2,L"Training. Model saved");
  }

  /////////////////////////////////////////////////////////////////////
  /// Use given text file to train a new model
  //////////////////////////////////////////////////////////////////////

  void idioma::create_model(wistream &f) {

    wchar_t c1,c2,c3,c4;

    TRACE(3,L"Training. Creating model");

    // initial trigram
    initial_trigram(f,c1,c2,c3);
    // initialize sequence probability
    increment(pi,trigram(c1,c2,c3));

    f>>noskipws>>c4; 
    c4=towlower(c4);
    while (not f.eof()) {
      TRACE(3,L"Current trigram: ("+to_writable(trigram(c1,c2,c3))+L")  next: ("+to_writable(wstring(1,c4))+L")");

      /// Count n-gram occurrence
      increment(tB,trigram(c1,c2,c3),wstring(1,c4));
      increment(bB,wstring(1,c2)+wstring(1,c3),wstring(1,c4));
      increment(uB,wstring(1,c3),wstring(1,c4));
      increment(A,wstring(1,c4));

      if (c4==L'\n') {
        // Found end-of-paragraph. 
        // Reset to init state for next paragraf
        initial_trigram(f,c1,c2,c3);   
        if (!f.eof()) {
          increment(pi,trigram(c1,c2,c3));
          f>>noskipws>>c4;        
          c4=towlower(c4);
        }
        TRACE(4,L"End of paragraph. Current trigram: ("+to_writable(trigram(c1,c2,c3))+L")  next: ("+to_writable(wstring(1,c4))+L")");
      }
      else { 
        // normal next, shift trigram window.
        c1=c2; c2=c3; c3=c4; 
        f>>noskipws>>c4;
        if (c3==L' ' || c3==L'\t') {
          while (!f.eof() && (c4==L' ' || c4==L'\t')) 
            f>>noskipws>>c4;
          TRACE(4,L"End of word. Skip whitespaces.");
        }
        c4=towlower(c4);
      }
    }
  }

  /////////////////////////////////////////////////////////////////////
  /// Initial trigram: two fictitious '/n' plus the first actual letter.
  //////////////////////////////////////////////////////////////////////

  void idioma::initial_trigram(wistream &f, wchar_t &x, wchar_t &y, wchar_t &z) const {
    x=L'\n'; 
    y=L'\n';
    f>>skipws>>z;
    z=towlower(z);
  }

  /////////////////////////////////////////////////////////////////////
  /// build actual trigram from iterators
  //////////////////////////////////////////////////////////////////////

  wstring idioma::trigram(wchar_t x, wchar_t y, wchar_t z) const {
    return wstring(1,x)+wstring(1,y)+wstring(1,z);
  }

  //////////////////////////////////////////////////////////////////////
  /// Find transition probability between two consecutive trigrams.
  //////////////////////////////////////////////////////////////////////

  double idioma::ProbA(const wstring &estat1, wchar_t nextch) const {

    map<wstring,double>::const_iterator k;
    double pa=-1E20; // Shouldn't be used, just to shut up compiler

    // use shorter n-grams until observations are found
    bool found=false;
    for (int n=0; n<3 and not found; n++) {
      TRACE(4,L"ProbA: Using "+util::int2wstring(4-n)+L"-grams. Search for "+to_writable(estat1.substr(n)+wstring(1,nextch)));
      k = pa_nom.find(estat1.substr(n)+wstring(1,nextch));
      if (k!=pa_nom.end()) {
        // seen n-gram transition, use stored value and exit loop
        pa = k->second;   
        found = true;
      }
    }

    if (not found)  {          
      // no n-grams found. Use unigram probability.
      TRACE(5,L"ProbA: Using unigrams.");
      k = pa_nom.find(wstring(1,nextch));
      if (k!=pa_nom.end()) 
        pa = k->second;   
      else {
        // no unigrams found. 
        TRACE(5,L"ProbA: back off failed. Smoothing.");
        pa = -15;
      }
    }
    
    TRACE(4,L"ProbA from ("+to_writable(estat1)+L") to ("+to_writable(wstring(1,nextch))+L") = "+util::double2wstring(pa));
    return pa;
  }

  //////////////////////////////////////////////////
  /// Find initial probability for a given trigram.
  //////////////////////////////////////////////////

  double idioma::ProbPi(const wstring &stat) const {
    double ppi;
  
    map<wstring,double>::const_iterator k = ppi_nom.find(stat);
    if (k!=ppi_nom.end()) 
      ppi = k->second;  // seen initial state
    else if (stat.find(L"\n\n")==0) {
      // unseen -but possible- initial state, since it's a paragraf start. 
      // Use unigram prob if possible
      k = pa_nom.find(stat.substr(2));
      if (k!=pa_nom.end()) 
        ppi = k->second;   
      else
        ppi = -10;
    }
    else
      ppi=-1E20;  // very unlikely an initial state. 

    TRACE(4,L"ProbPi for ("+to_writable(stat)+L") ="+util::double2wstring(ppi));
    return ppi;
  }

  //////////////////////////////////////////////////
  /// Increment counts of occurrences of trigram k.
  ////////////////////////////////////////////////////

  void idioma::increment(map<wstring,double> &m, const wstring &k, double n) {
    map<wstring,double>::iterator p = m.find(k);
    if (p==m.end()) m.insert(make_pair(k,n));
    else p->second += n;                 
  }

  ///////////////////////////////////////////////////////////////////////
  /// Increment counts of occurrences of trigram k2 following trigram k1
  ////////////////////////////////////////////////////////////////////////

  void idioma::increment(map<pair<wstring,wstring>,double> &m, const wstring &k1, const wstring &k2, double n) {
    map<pair<wstring,wstring>,double>::iterator p = m.find(make_pair(k1,k2));
    if (p==m.end()) m.insert(make_pair(make_pair(k1,k2),n));
    else p->second += n;
  }

  ///////////////////////////////////////////////////////////////////////
  /// Convert an ascii string with newlines and whitespaces to 
  /// something easily writable
  ///////////////////////////////////////////////////////////////////////

  wstring idioma::to_writable(const wstring &s) const {
    wstring x;
    for (unsigned int i=0; i<s.size(); i++) {
      if (s[i]==L'\n') x = x+L"\\n";
      else if (s[i]==L' ') x = x+L"\\s";
      else if (s[i]==L'\\') x = x+L"\\\\";
      else x = x+wstring(1,s[i]);
    }
  
    return x;
  } 

  ///////////////////////////////////////////////////////////////////////
  /// Convert a string with newlines and whitespaces marked to an 
  /// actual ascii string
  ///////////////////////////////////////////////////////////////////////

  wstring idioma::from_writable(const wstring &s) const {
    wstring x;
    for (unsigned int i=0; i<s.size(); i++) {
      if (s[i]==L'\\' && s[i+1]==L'n') { x = x+wstring(1,L'\n'); i++; }
      else if (s[i]==L'\\' && s[i+1]==L's') { x = x+wstring(1,L' '); i++; }
      else if (s[i]==L'\\' && s[i+1]==L'\\') { x = x+wstring(1,L'\\'); i++; }
      else x = x+wstring(1,s[i]);
    }

    return x;
  } 

} // namespace

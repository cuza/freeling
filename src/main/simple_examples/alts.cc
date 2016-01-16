#include <iostream>
#include "freeling.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"

using namespace std;

#define MOD_TRACECODE CORRECTOR_TRACE
#define MOD_TRACENAME L"NORMALITZADOR"

///---------------------------------------------
/// Analyzers
///---------------------------------------------

/////////////////////////////////////////////////////////////////////
/////////  
/////////                 MAIN PROGRAM 
/////////  
/////////////////////////////////////////////////////////////////////

int main (int argc, char **argv) {

  /// set locale to an UTF8 compatible locale
  freeling::util::init_locale(L"default");

  /// path to freeling installation to be used
  wstring ipath;
  if (argc < 2) ipath=L"/usr/local";
  else ipath=freeling::util::string2wstring(argv[1]);
  /// path to data files
  wstring path=ipath+L"/share/freeling/";

  // set the language to your desired target
  const wstring lang=L"es";

  // if FreeLing was compiled with --enable-traces, you can activate
  // the required trace verbosity for the desired modules.
  // freeling::traces::TraceLevel=6;
  // freeling::traces::TraceModule=0x0E800000;
  
  // create modules
  // create analyzers
  freeling::tokenizer tk(path+lang+L"/tokenizer.dat"); 
  freeling::splitter sp(path+lang+L"/splitter.dat");
  
  // morphological analysis has a lot of options, and for simplicity they are packed up
  // in a maco_options object. First, create the maco_options object with default values.
  freeling::maco_options opt(lang);  

  // then, set required options on/off  
  opt.UserMap=false;
  opt.QuantitiesDetection = true; 
  opt.AffixAnalysis = true; 
  opt.MultiwordsDetection = true;
  opt.NumbersDetection = true; 
  opt.PunctuationDetection = true; opt.DatesDetection = true; opt.QuantitiesDetection = false; 
  opt.DictionarySearch = true; opt.ProbabilityAssignment = false; opt.NERecognition = true;   

  // and provide files for morphological submodules. Note that it is not necessary
  // to set opt.QuantitiesFile, since Quantities module was deactivated.
  opt.UserMapFile=L"";
  opt.QuantitiesFile=path+lang+L"/quantities.dat";
  opt.LocutionsFile=path+lang+L"/locucions.dat"; opt.AffixFile=path+lang+L"/afixos.dat";
  opt.ProbabilityFile=path+lang+L"/probabilitats.dat"; opt.DictionaryFile=path+lang+L"/dicc.src";
  opt.NPdataFile=path+lang+L"/np.dat"; opt.PunctuationFile=path+L"/common/punct.dat"; 

  // create the analyzer with the just build set of maco_options
  freeling::maco morfo(opt); 

  // create alternative porposers
  freeling::alternatives alts_ort(path+lang+L"/alternatives-ort.dat");

  // IMPORTANT: comment this out if there is no phonetic encoder for target language
  freeling::alternatives alts_phon(path+lang+L"/alternatives-phon.dat");

  // get plain text input lines while not EOF.
  wstring text;
  list<freeling::word> lw;
  list<freeling::sentence> ls;
  while (getline(wcin,text)) {
    
    // tokenize input line into a list of words
    lw=tk.tokenize(text);
    // split into sentences, flushing buffer at each line
    ls=sp.split(lw, true);    
    // perform morphosyntactic analysis 
    morfo.analyze(ls);

    // propose alternative forms
    alts_ort.analyze(ls);

    // IMPORTANT: comment this out if there is no phonetic encoder for your language    
    alts_phon.analyze(ls);

    // print results.
    for (list<freeling::sentence>::iterator s=ls.begin(); s!=ls.end(); s++) {
      for (freeling::sentence::iterator w=s->begin(); w!=s->end(); w++) {
        wcout<<L"FORM: "<<w->get_form()<<endl; 
        wcout<<L"   ANALYSIS:";
        for (freeling::word::iterator a=w->begin(); a!=w->end(); a++) 
          wcout<<L" ["<<a->get_lemma()<<L","<<a->get_tag()<<L"]";
        wcout<<endl;
        wcout<<L"   ALTERNATIVE FORMS:";
        for (list<pair<wstring,int> >::iterator a=w->alternatives_begin(); a!=w->alternatives_end(); a++) 
           wcout<<L" ["<<a->first<<L","<<a->second<<L"]";
        wcout<<endl; 
      }
      wcout<<endl;
    }

    // clear temporary lists;
    lw.clear(); ls.clear();    
  }
  
}


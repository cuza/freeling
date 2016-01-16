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
#include "freeling/morfo/maco.h"

using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"MACO"
#define MOD_TRACECODE MACO_TRACE

  ///////////////////////////////////////////////////////////////
  ///  Create the morphological analyzer, and all required 
  /// recognizers and modules.
  ///////////////////////////////////////////////////////////////

  maco::maco(const maco_options &opts): defaultOpt(opts) {

    user = (opts.UserMap               ? new RE_map(opts.UserMapFile) 
            : NULL);
    numb = (opts.NumbersDetection      ? new numbers(opts.Lang,opts.Decimal,opts.Thousand) 
            : NULL);
    punt = (opts.PunctuationDetection  ? new punts(opts.PunctuationFile) 
            : NULL);
    date = (opts.DatesDetection        ? new dates(opts.Lang) 
            : NULL);
    dico = (opts.DictionarySearch      ? new dictionary(opts.Lang, opts.DictionaryFile, 
                                                        opts.AffixAnalysis, opts.AffixFile,
                                                        opts.InverseDict, opts.RetokContractions) 
            : NULL);
    loc = (opts.MultiwordsDetection    ? new locutions(opts.LocutionsFile) 
           : NULL);

    npm = (opts.NERecognition          ? new ner(opts.NPdataFile) 
           : NULL);

    quant = (opts.QuantitiesDetection  ? new quantities(opts.Lang, opts.QuantitiesFile)
             : NULL);
    prob = (opts.ProbabilityAssignment ? new probabilities(opts.ProbabilityFile, 
                                                           opts.ProbabilityThreshold)
            : NULL);
  }

  ///////////////////////////////////////////////////////////////
  ///  Destroy morphological analyzer, and all required 
  /// recognizers and modules.
  ///////////////////////////////////////////////////////////////

  maco::~maco() {
    delete user;
    delete numb;
    delete punt;
    delete date;
    delete dico;
    delete loc;
    delete npm;
    delete quant;
    delete prob;
  }


  ///////////////////////////////////////////////////////////////
  ///  Apply cascade of analyzers to given sentence.
  ///////////////////////////////////////////////////////////////  

  void maco::analyze(sentence &s) const {
  
    if (defaultOpt.UserMap){ 
      user->analyze(s);
      TRACE(2,L"Sentence annotated by the user-map module.");
    }

    if (defaultOpt.NumbersDetection){ 
      // (Skipping number detection will affect dates and quantities modules)
      numb->analyze(s);    
      TRACE(2,L"Sentence annotated by the numbers module.");
    }

    if(defaultOpt.PunctuationDetection){ 
      punt->analyze(s);    
      TRACE(2,L"Sentence annotated by the punts module.");
    }
     
    if(defaultOpt.DatesDetection) { 
      date->analyze(s);    
      TRACE(2,L"Sentence annotated by the dates module.");
    }

    if(defaultOpt.DictionarySearch) {
      // (Skipping dictionary search will also skip suffix analysis)
      dico->analyze(s);
      TRACE(2,L"Sentence annotated by the dictionary searcher.");
    }

    // annotate list of sentences with required modules
    if (defaultOpt.MultiwordsDetection) { 
      loc->analyze(s);
      TRACE(2,L"Sentence annotated by the locutions module.");
    }

    if (defaultOpt.NERecognition) { 
      npm->analyze(s);
      TRACE(2,L"Sentence annotated by the np module.");
    }

    if(defaultOpt.QuantitiesDetection){
      quant->analyze(s);    
      TRACE(2,L"Sentence annotated by the quantities module.");
    }

    if (defaultOpt.ProbabilityAssignment) {
      prob->analyze(s);    
      TRACE(2,L"Sentences annotated by the probabilities module.");
    }

    // mark all analysis of each word as selected (taggers assume it's this way)
    for (sentence::iterator w=s.begin(); w!=s.end(); w++)
      w->select_all_analysis();
  }

} // namespace

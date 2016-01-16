
#include <ctime>
#include <iostream>
#include "freeling.h"
#include "test_speed_config.h"

using namespace std;

double seconds(clock_t tmp) {
  return (double)tmp/(double)CLOCKS_PER_SEC;
}

int main(int argc, char* argv[]) {
  
  clock_t b0,bini,bend;

  if (argc<2) {
    wcerr<<endl;
    wcerr<<L"Usage: test-speed -f file.cfg  < mytext.txt "<<endl;
    wcerr<<endl;
    wcerr<<L"  Where:  "<<endl;
    wcerr<<L"     - file.cfg is the absolute path to a FreeLing configuration file"<<endl;
    wcerr<<L"     - mytext.txt is a file containing plain text. The file should contain at"<<endl;
    wcerr<<L"       least a few thousands words to obtain reliable measurements."<<endl;
    wcerr<<endl;
    wcerr<<L"  You will need to define $FREELINGSHARE to point to the share/freeling directory"<<endl;
    wcerr<<L"  of your FreeLing installation."<<endl;
    wcerr<<endl;
    exit(1);
  }

  bini=clock();

  config *cfg = new config(argc,argv);

  b0=clock();
  freeling::util::init_locale(cfg->Locale);
  freeling::tokenizer tk(cfg->TOK_TokenizerFile); 
  freeling::splitter sp(cfg->SPLIT_SplitterFile); 
  freeling::maco_options opt(cfg->Lang);
  opt.set_active_modules (cfg->MACO_UserMap, cfg->MACO_AffixAnalysis, cfg->MACO_MultiwordsDetection,
                            cfg->MACO_NumbersDetection, cfg->MACO_PunctuationDetection,
                            cfg->MACO_DatesDetection, cfg->MACO_QuantitiesDetection,
                            cfg->MACO_DictionarySearch, cfg->MACO_ProbabilityAssignment,
                            cfg->MACO_NERecognition, cfg->MACO_OrthographicCorrection);
  opt.set_nummerical_points (cfg->MACO_Decimal, cfg->MACO_Thousand);
  opt.set_threshold (cfg->MACO_ProbabilityThreshold);
  opt.set_inverse_dict(false);
  opt.set_retok_contractions(cfg->MACO_RetokContractions);
  opt.set_data_files (cfg->MACO_UserMapFile, cfg->MACO_LocutionsFile, cfg->MACO_QuantitiesFile,
                      cfg->MACO_AffixFile, cfg->MACO_ProbabilityFile, cfg->MACO_DictionaryFile, 
	              cfg->MACO_NPDataFile, cfg->MACO_PunctuationFile,cfg->MACO_CorrectorFile);
  freeling::maco morfo(opt);
  freeling::hmm_tagger tagger(cfg->TAGGER_HMMFile, cfg->TAGGER_Retokenize, cfg->TAGGER_ForceSelect);

  freeling::senses *sens=NULL;
  freeling::ukb *wsd=NULL;
  if (cfg->SENSE_WSD_which == UKB) {
    sens = new freeling::senses(cfg->SENSE_ConfigFile);
    wsd = new freeling::ukb(cfg->UKB_ConfigFile);
  }

  freeling::chart_parser parser(cfg->PARSER_GrammarFile);
  freeling::dep_txala dep(cfg->DEP_TxalaFile, parser.get_start_symbol ());
  double timeinit = clock()-b0;

  list<freeling::word> lw;
  list<freeling::sentence> ls;
  int nw=0,ns=0;
  wstring text;
  double timetok=0, timesplit=0, timemorfo=0, timetag=0, timesenses=0, timeukb=0, timeparser=0, timedep=0;
  while (getline(wcin,text)) {
    b0=clock();
    lw=tk.tokenize(text);
    timetok += clock()-b0;

    b0=clock();
    ls=sp.split(lw, false);
    timesplit += clock()-b0;

    b0=clock();
    morfo.analyze(ls);
    timemorfo += clock()-b0;
	
    b0=clock();
    tagger.analyze(ls);
    timetag += clock()-b0;

    if (cfg->SENSE_WSD_which == UKB) {
      b0=clock();
      sens->analyze(ls);
      timesenses += clock()-b0;
      
      b0=clock();
      wsd->analyze(ls);
      timeukb += clock()-b0;
    }

    b0=clock();
    parser.analyze(ls);
    timeparser += clock()-b0;

    b0=clock();
    dep.analyze(ls);
    timedep += clock()-b0;

    for (list<sentence>::iterator s=ls.begin(); s!=ls.end(); s++) {
      ns++;
      nw+=s->size();
    }
  }

  bend=clock();

  wcerr<<L"Analyzed "<<nw<<" words in "<<ns<<" sentences."<<endl;
  wcerr<<L"Time (seconds) spent in: "<<endl;
  wcerr<<L"  Creating modules: "<<seconds(timeinit)<<endl;
  wcerr<<L"  Tokenizing = "<<seconds(timetok)<<" ("<<nw/seconds(timetok)<<" w/s)"<<endl;
  wcerr<<L"  Splitting = "<<seconds(timesplit)<<" ("<<nw/seconds(timesplit)<<" w/s)"<<endl;
  wcerr<<L"  Morfing = "<<seconds(timemorfo)<<" ("<<nw/seconds(timemorfo)<<" w/s)"<<endl;
  wcerr<<L"  Tagging = "<<seconds(timetag)<<" ("<<nw/seconds(timetag)<<" w/s)"<<endl;
  if (cfg->SENSE_WSD_which == UKB) {
    wcerr<<L"  Annotating senses = "<<seconds(timesenses)<<" ("<<nw/seconds(timesenses)<<" w/s)"<<endl;
    wcerr<<L"  Disambiguating senses = "<<seconds(timeukb)<<" ("<<nw/seconds(timeukb)<<" w/s)"<<endl;
  }
  wcerr<<L"  Chunking = "<<seconds(timeparser)<<" ("<<nw/seconds(timeparser)<<" w/s)"<<endl;
  wcerr<<L"  Dep-parsing = "<<seconds(timedep)<<" ("<<nw/seconds(timedep)<<" w/s)"<<endl;

  double fl_time=timetok+timesplit+timemorfo+timetag+timesenses+timeukb+timeparser+timedep;
  wcerr<<L"  TOTAL FreeLing services = "<<seconds(fl_time)<<" ("<<nw/seconds(fl_time)<<" w/s)"<<endl;

  wcerr<<L"TOTAL execution= "<<seconds(bend-bini)<<endl;

  delete sens; delete wsd;
}


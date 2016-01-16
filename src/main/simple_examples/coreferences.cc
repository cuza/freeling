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

#include "freeling.h"

using namespace std;
using namespace freeling;


void OutputMySenses(const analysis &a) {
  const list<pair<wstring,double> > & ls=a.get_senses();
  if (ls.size()>0) {
    wcout<<L" "<<util::pairlist2wstring(ls,L":",L"/");
  }
  else wcout<<L" -";
}
//---------------------------------------------
// print obtained analysis.
//---------------------------------------------
void PrintMyTree(document & doc, parse_tree::iterator n, int depth) {
  parse_tree::sibling_iterator d;
  
  wcout<<wstring(depth*2,' ');
  if (n->num_children()==0) {
    if (n->info.is_head()) { wcout<<L"+";}
    const word & w=n->info.get_word();
    wcout<<L"("<<w.get_form()<<L" "<<w.get_lemma()<<L" "<<w.get_tag();
    OutputMySenses((*w.selected_begin()));
    wcout<<L")"<<endl;
  } 
  else {
    if (n->info.is_head()) { wcout<<L"+";}

    //Get the class of coreference and print.
    int ref = doc.get_coref_group(n->info.get_node_id());
    if(n->info.get_label() == L"sn" && ref != -1)
      wcout<<n->info.get_label()<<L"(REF " << ref <<L")_["<<endl;
    else 
      wcout<<n->info.get_label()<<L"_["<<endl;
    
    for (d=n->sibling_begin(); d!=n->sibling_end(); ++d)
      PrintMyTree(doc, d, depth+1);
    wcout<<wstring(depth*2,' ')<<L"]"<<endl;
  }
}

int main (int argc, char **argv) {
  wstring text;
  list<word> lw;
  list<sentence> ls;
  paragraph par;
  document doc;
  
  /// set locale to an UTF8 compatible locale
  util::init_locale(L"default");
  
  wstring ipath;
  if(argc < 2) ipath=L"/usr/local";
  else ipath=util::string2wstring(argv[1]);

  wstring path=ipath+L"/share/freeling/es/";
  
  // create analyzers
  tokenizer tk(path+L"tokenizer.dat");
  splitter sp(path+L"splitter.dat");
  
  // morphological analysis has a lot of options, and for simplicity they are packed up
  // in a maco_options object. First, create the maco_options object with default values.
  maco_options opt(L"es");
  // enable/disable desired modules
  opt.set_active_modules(false, true, true, true, true, true, true, true, true, true, false);

  // provide config files for each module
  opt.set_data_files(L"",path+L"locucions.dat",path+L"quantities.dat",path+L"afixos.dat",
                     path+L"probabilitats.dat",path+L"dicc.src",path+L"np.dat",
                     path+L"/../common/punct.dat",L"");  

  // create the analyzer with the just build set of maco_options
  maco morfo(opt);
  
  // create a hmm tagger for spanish (with retokenization ability, and forced
  // to choose only one tag per word)
  hmm_tagger tagger(path+L"tagger.dat", true, true);
  // create a shallow parser
  chart_parser parser(path+L"grammar-dep.dat");
  
  // create a NE classifier
  nec *neclass = new nec(path+L"nec/nec-ab.dat");
  
  // create a correference solver
  coref *corefclass = new coref(path+L"coref/coref.dat)");
  
  // get plain text input lines while not EOF.
  while (getline(wcin,text)) {
    // clear temporary lists;
    lw.clear();
    ls.clear();
    
    // tokenize input line into a list of words
    lw=tk.tokenize(text);
    // split on sentece boundaries
    ls=sp.split(lw, false);
    // build paragraphs from sentences
    par.insert(par.end(), ls.begin(), ls.end());
  }
  lw.clear();
  
  ls=sp.split(lw, true);
  par.insert(par.end(), ls.begin(), ls.end());
  
  // build document from paragraphs
  doc.push_back(par);
  
  // IMPORTANT : In this simple example, each document is assumed to consist of only one paragraph.
  // if your document has more than one, you'll have to loop to process them all. 
  
  // Process text in the document up to shallow parsing
  morfo.analyze(doc.front());
  tagger.analyze(doc.front());
  neclass->analyze(doc.front());
  parser.analyze(doc.front());
  
  // HERE you would end the loop, to get all the document parsed
  
  // then, try to solve correferences in the document
  corefclass->analyze(doc);
  
  // for each sentence in each paragraph, output the tree and coreference information.
  list<paragraph>::iterator parIt;
  list<sentence>::iterator seIt;
  for (parIt = doc.begin(); parIt != doc.end(); ++parIt){
    for(seIt = (*parIt).begin(); seIt != (*parIt).end(); ++seIt){
      PrintMyTree(doc, (*seIt).get_parse_tree().begin(), 0);
    }
  }
}



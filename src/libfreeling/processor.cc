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

#include "freeling/morfo/processor.h"

using namespace std;

namespace freeling {

  ///////////////////////////////////////////////////////////////
  /// Constructor
  ///////////////////////////////////////////////////////////////

  processor::processor() {};

  ////////////////////////////////////////////////////////////////
  /// Analyze given sentences 
  ////////////////////////////////////////////////////////////////

  void processor::analyze(list<sentence> &ls) const {
    list<sentence>::iterator is;
    for (is=ls.begin(); is!=ls.end(); is++) {
      analyze(*is);    
    }
  }


  ////////////////////////////////////////////////////////////////
  /// Analyze given sentence, return copy
  ////////////////////////////////////////////////////////////////

  sentence processor::analyze(const sentence &s) const {
    sentence s2=s;
    analyze(s2);    
    return s2;
  }

  ////////////////////////////////////////////////////////////////
  /// Analyze given sentences, return copy
  ////////////////////////////////////////////////////////////////

  list<sentence> processor::analyze(const list<sentence> &ls) const {
    list<sentence> l2=ls;
    analyze(l2);
    return l2;
  }
} // namespace

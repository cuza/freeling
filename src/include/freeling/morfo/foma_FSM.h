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

#include <string>

typedef bool _Bool;
#include "freeling/foma/fomalib.h"


////////////////////////////////////////////////////////
///  Class foma_FSM is a wrapper for the FOMA library, 
///  for the specific use of getting entries from a 
///  dictionary with minimum edit distance to given key
////////////////////////////////////////////////////////
namespace freeling {

  class WINDLL foma_FSM {
  private:
    /// foma automaton
    struct fsm *fsa;
    /// Handle for foma minimum edit distance automaton
    struct apply_med_handle *h_fsa;

  public:
    /// build automaton from text file
    foma_FSM(const std::wstring &, const std::wstring &mcost=L""); 
    /// clear 
    ~foma_FSM();

    /// Use automata to obtain closest matches to given form, and add them to given list.
    void get_similar_words(const std::wstring &, std::list<std::pair<std::wstring,int> > &) const;    
    /// set maximum edit distance of desired results
    void set_cutoff_threshold(int);
    /// set maximum number of desired results
    void set_num_matches(int);
    /// Set cost for basic SED operations
    void set_basic_operation_cost(int);
  };

}

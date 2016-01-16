////////////////////////////////////////////////////////////////
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

#ifndef _PROCESSOR
#define _PROCESSOR

#include <set>

#include "freeling/windll.h"
#include "freeling/morfo/language.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///
  ///   Abstract class to define the common API of any
  ///  FreeLing processing module.
  ///
  //////////////////////////////////////////////////////////////////

  class WINDLL processor {
  public:
    /// constructor
    processor();
    /// destructor
    virtual ~processor() {};

    /// analyze sentence
    virtual void analyze(sentence &) const =0;
    /// analyze sentences
    virtual void analyze(std::list<sentence> &) const;
    /// analyze sentence, return analyzed copy
    virtual sentence analyze(const sentence &) const;
    /// analyze sentences, return analyzed copy
    virtual std::list<sentence> analyze(const std::list<sentence> &) const;
  };

} // namespace

#endif

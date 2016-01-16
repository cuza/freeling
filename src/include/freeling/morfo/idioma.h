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

///////////////////////////////////
//
//  idioma.h
//
//  Class that implements a Visible Markov Model 
//  to compute the probability that a text is 
//  written in a certain language.
// 
///////////////////////////////////

#ifndef _IDIOMA_H
#define _IDIOMA_H

#include <map>
#include <list>
#include <vector>
#include <string>

namespace freeling {

  ///////////////////////////////////
  /// Class "idioma" implements a visible Markov's model that calculates
  /// the probability that a text is in a certain language.
  ///////////////////////////////////

  class idioma {  

  private:
    // language code
    std::wstring LangCode;
    /// State transitions probabilities
    std::map<std::wstring,double> pa_nom;
    /// Initial probabilities
    std::map<std::wstring,double> ppi_nom;

    /// auxiliary for training 
    std::map<std::wstring,double> pi,A;
    /// auxiliary for training
    std::map<std::pair<std::wstring,std::wstring>,double> tB,bB,uB;

    /// scale factor to apply to resulting probability (useful to 
    /// equalize models among languages)
    double scale;

    /// convert a trigram from writable represntation in the model file
    std::wstring from_writable(const std::wstring &) const;
    /// convert a trigram to a writable represntation for the model file
    std::wstring to_writable(const std::wstring &) const;
    /// Consult method for transition probabilities
    double ProbA(const std::wstring &, wchar_t) const;
    /// Consult method for initial probabilities
    double ProbPi(const std::wstring &) const; 
    /// Increase occurrences of a n-gram
    void increment(std::map<std::wstring,double> &, const std::wstring &, double n=1.0);
    /// Increase occurrences of a two chained trigrams
    void increment(std::map<std::pair<std::wstring,std::wstring>,double> &, const std::wstring &, const std::wstring &, double n=1.0);
    /// Initial trigram: two fictitious '\n' plus the first actual letter.
    void initial_trigram(std::wistream &, wchar_t &, wchar_t &, wchar_t &) const;
    /// build actual trigram from iterators
    std::wstring trigram(wchar_t, wchar_t, wchar_t) const;
    /// Create new model from given stream, with given language code.
    void create_model(std::wistream &f);
    /// Save current model in given file
    void save_model(const std::wstring &) const;
 
  public:
    /// null constructor
    idioma();
    /// Constructor, given the model file to load
    idioma(const std::wstring &);
    /// Calculates the probability that the text is in the instance language.
    double sequence_probability(std::wistream &, size_t &) const;
    /// Compute normalized language probability for given string
    double compute_probability(const std::wstring &, double s=1.0) const; 
    /// Create a new model for the language from given input file,
    /// Store model in given filename, with given language code.
    void train(const std::wstring &, const std::wstring &, const std::wstring&);
    void train(std::wistream &f, const std::wstring &, const std::wstring&);
    /// get iso code for current language
    std::wstring get_language_code() const;
  };

} // namespace

#endif


#include <fstream>  
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/tagset.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

#define MOD_TRACENAME L"TAGSET"
#define MOD_TRACECODE TAGGER_TRACE

using namespace std;

namespace freeling {

  //////////////////////////////////////////////////////////////////////
  /// --- constructor: load given file
  //////////////////////////////////////////////////////////////////////

  tagset::tagset(const wstring &ftagset) : PAIR_SEP(L"="), MSD_SEP(L"|") {

    // configuration file
    enum sections {DIRECT_TRANSLATIONS, DECOMPOSITION_RULES};
    config_file cfg;  
    cfg.add_section(L"DirectTranslations",DIRECT_TRANSLATIONS);
    cfg.add_section(L"DecompositionRules",DECOMPOSITION_RULES);
  
    if (not cfg.open(ftagset))
      ERROR_CRASH(L"Error opening file "+ftagset);

    wstring line;
    while (cfg.get_content_line(line)) {        

      switch (cfg.get_section()) {
      case DIRECT_TRANSLATIONS: {
        // reading direct tag translation
        wistringstream sin;
        sin.str(line);
        wstring tag, shtag, msf;
        sin>>tag>>shtag>>msf;
        direct[tag] = make_pair(shtag,msf); 
        TRACE(4,L"Read direct translation for "+tag+L" = ("+shtag+L","+msf+L")");
        break;
      }
    
      case DECOMPOSITION_RULES: { 
        // reading a decomposition rule
        wistringstream sin;
        sin.str(line);
        wstring cat, msf; int shsz; 
        
        // read category and short tag size
        sin>>cat>>shsz;
        shtag_size[cat] = shsz;
        
        TRACE(4,L"Read short tag size for "+cat+L" = "+util::int2wstring(shsz));
        
        // other fields are features
        int i=1;
        while (sin>>msf) {
          wstring key = cat+L"#"+util::int2wstring(i);
          
          vector<wstring> k=util::wstring2vector(msf,L"/");  // separate feature name/values "postype/C:common;P:proper"
          feat[key] = k[0];    // store name (e.g. "postype")
          vector<wstring> v=util::wstring2vector(k[1],L";");  // separate values "C:common;P:proper"
          for (size_t j=0; j<v.size(); j++) {
            vector<wstring> t=util::wstring2vector(v[j],L":"); // split value code:name "C:common"
            val[key+L"#"+t[0]] = t[1];
          }
          
          i++;
        }
        break;
      }

      default: break;
      }
    }

    cfg.close();

    TRACE(1,L"Module created successfully.");
  }


  //////////////////////////////////////////////////////////////////////
  /// --- destructor
  //////////////////////////////////////////////////////////////////////

  tagset::~tagset() {};



  //////////////////////////////////////////////////////////////////////
  /// get short version of given tag
  //////////////////////////////////////////////////////////////////////

  wstring tagset::get_short_tag(const wstring &tag) const {

    TRACE(5,L"get short tag for "+tag);
    // if direct translation exists, get it 
    map<wstring,pair<wstring,wstring> >::const_iterator p=direct.find(tag);
    if (p!=direct.end()) {
      TRACE(5,L"  Found direct entry "+p->second.first);
      return p->second.first;
    }
    else {
      // no direct value, compute short tag cutting n first positions (n==0 -> all)
      map<wstring,int>::const_iterator s=shtag_size.find(tag.substr(0,1));
      if (s!=shtag_size.end()) {
        TRACE(5,L"   cuting first positions sz="+util::int2wstring(s->second)+L" of "+tag);
        return (s->second==0 ? tag : tag.substr(0,s->second));
      }
    }

    // we don't know anything about this tag
    WARNING(L"No rule to get short version of tag '"+tag+L"'.");
    return tag;
  }



  //////////////////////////////////////////////////////////////////////
  /// get list of <feature,value> pairs with morphological information
  //////////////////////////////////////////////////////////////////////

  list<pair<wstring,wstring> > tagset::get_msf_features(const wstring &tag) const {

    TRACE(5,L"get msf for "+tag);
    map<wstring,pair<wstring,wstring> >::const_iterator p=direct.find(tag);
    if (p!=direct.end())
      return util::wstring2pairlist(p->second.second,PAIR_SEP,MSD_SEP);
    else 
      return compute_msf_features(tag);
  }


  //////////////////////////////////////////////////////////////////////
  /// get list <feature,value> pairs with morphological information, in a string format
  //////////////////////////////////////////////////////////////////////

  wstring tagset::get_msf_string(const wstring &tag) const {

    TRACE(5,L"get msf string for "+tag);
    map<wstring,pair<wstring,wstring> >::const_iterator p=direct.find(tag);
    if (p!=direct.end()) 
      return p->second.second;
    else
      return util::pairlist2wstring(compute_msf_features(tag),PAIR_SEP,MSD_SEP);
  }


  //////////////////////////////////////////////////////////////////////
  /// private method to decompose the tag into morphological features
  /// interpreting each digit in the tag according to field definition.
  /// feat[<cat,i>] is the feature name (e.g. "postype")
  /// val[<cat,i>] is a map<code,name> with the feature values
  //////////////////////////////////////////////////////////////////////

  list<pair<wstring,wstring> > tagset::compute_msf_features(const wstring &tag) const {

    TRACE(5,L"computing msf for "+tag);
    wstring cat = tag.substr(0,1);  // get category
    list<pair<wstring,wstring> > res;
    for (size_t i=1; i<tag.size(); i++) {
      wstring key = cat+L"#"+util::int2wstring(i); // field number (e.g N#2 -> 2nd field of a "N" tag)
      wstring code = tag.substr(i,1);  // feature code
      map<wstring,wstring>::const_iterator f=feat.find(key); // feature name

      if (f==feat.end()) return res;  // position description not found. There is no more information about this tag.
    
      wstring featname = f->second;
      if (code!=L"0") {
        // retrieve values for field+code (e.g. N#2#M -> "M" code found in 2nd field of "N" tag is translated, e.g, as "masc")
        map<wstring,wstring>::const_iterator v = val.find(key+L"#"+code);  
        if (v==val.end()) {
          // no translation found. Invalid PoS tag.
          WARNING(L"Tag "+tag+L": Invalid code '"+code+L"' for feature '"+featname+L"'");
        }
        else {
          // Translation found. Output it if not "0" (0 -> "ignore")
          if (v->second!=L"0")
            res.push_back(make_pair(featname,v->second));
        }
      }
    }

    return res;
  }
} // namespace

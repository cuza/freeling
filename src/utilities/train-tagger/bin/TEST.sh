#! /bin/bash

## Test the performance of a PoS tagger
## See README file for details
##
##  Usage:
##    ./TEST.sh lang
##

# FreeLing installation. Adjust this path if needed,
# or call the script with: FLDIR=/my/FL/path ./TEST.sh lang
if ( test "x$FLDIR" = "x" ); then
  FLDIR=/usr/local
fi

export LD_LIBRARY_PATH=$FLDIR/lib
export PERL_UNICODE=SDL

# script location
BINDIR=$(cd $(dirname $0) && echo $PWD)

## language code for FreeLing (es,en,ru)...
## A directory with this name must exists and 
## contain a "train" and a "test" corpus.
LG=$1

if (test "#$LG" == "#en"); then 
  EAGLES=0
else 
  EAGLES=1
fi

## get locale for current language from config file
loc=`cat $FLDIR/share/freeling/config/$LG.cfg | grep 'Locale=' | cut -d'=' -f2`

## copy tagset to test directory (required by tagger.dat and probabiltites.dat)
cp $FLDIR/share/freeling/$LG/tagset.dat $LG/

# test HMM tagger
echo "Testing HMM"
cat $LG/test | $BINDIR/update-probs $LG 0.001 $LG/probabilitats.dat $loc |  gawk '{if (NF==0) print ""; else {$2="";$3="";$4=""; gsub("[ ]+"," "); print} }' | $FLDIR/bin/analyze -f $LG.cfg --inpf morfo --nortk -H $LG/tagger.dat > $LG/out-hmm.tmp
cut -d' ' -f1-3 <$LG/test | paste - $LG/out-hmm.tmp | $BINDIR/count.perl $LG $EAGLES 

# test relax tagger, with different models
for c in B T BT; do
  echo "Testing relax-"$c
  cat $LG/test | $BINDIR/update-probs $LG 0.001 $LG/probabilitats.dat $loc |  gawk '{if (NF==0) print ""; else {$2="";$3="";$4=""; gsub("[ ]+"," "); print} }' | $FLDIR/bin/analyze -f $LG.cfg --inpf morfo --nortk -t relax -R $LG/constr_gram-$c.dat > $LG/out-rlx$c.tmp
  cut -d' ' -f1-3 <$LG/test | paste - $LG/out-rlx$c.tmp | $BINDIR/count.perl $LG $EAGLES 
done

rm -f $LG/*.tmp $LG/tagset.dat

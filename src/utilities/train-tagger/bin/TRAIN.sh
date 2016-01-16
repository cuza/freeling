#! /bin/bash

## Train a PoS tagger and evaluate its performance.
## See README file for details
##
##  Usage:
##    ./TRAIN.sh lang
##

# FreeLing installation. Adjust this path if needed
FLDIR=/usr/local

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

if (test "#$LG" == "#ru"); then 
  THR_SUF=5
  THR_FORM=2
  THR_CLAS=4
else 
  THR_SUF=2
  THR_FORM=4
  THR_CLAS=8
fi

# Create lexical probabilites file
echo "Creating probabilities file"
cat $LG/train | gawk 'NF>0' | gawk '{bo=$2"#"$3; printf "%s %s %s",$1,$2,$3; for (i=5;i<=NF; i+=3) {if (bo!=$i"#"$(i+1)) printf " %s %s",$i,$(i+1);} printf "\n"}' | $BINDIR/make-probs-file.perl $LG $EAGLES $THR_SUF $THR_FORM $THR_CLAS 2>$LG/err.tmp > $LG/probabilitats.dat
# Report inconsistencies corpus-dictionary
cat $LG/err.tmp | sort | uniq -c | egrep -v '[ \-]N?NP' | sort -nr -k 1 >$LG/report.txt

# train hmm tagger
echo "Training HMM"
cat $LG/train | cut -d' ' -f1-3 | gawk 'NF>0' | $BINDIR/hmm_smooth.perl $LG $EAGLES  | cat $LG/hmm-forbidden.manual - > $LG/tagger.dat

#train relax tagger
echo "Training relax"
cat $LG/train | cut -d' ' -f1-3 | $BINDIR/train-relax-B.perl $LG $EAGLES >$LG/constr_gram-B.tmp
cat $LG/train | cut -d' ' -f1-3 | $BINDIR/train-relax-T.perl $LG $EAGLES >$LG/constr_gram-T.tmp
cat $LG/constr_gram.manual $LG/constr_gram-B.tmp > $LG/constr_gram-B.dat
cat $LG/constr_gram.manual $LG/constr_gram-T.tmp > $LG/constr_gram-T.dat
cat $LG/constr_gram.manual $LG/constr_gram-B.tmp $LG/constr_gram-T.tmp > $LG/constr_gram-BT.dat

rm -f $LG/*.tmp

#! /usr/bin/perl


use strict;

use File::Spec::Functions qw(rel2abs);
use File::Basename;

my $path = dirname(rel2abs($0));
require $path."/short_tag.perl";

my $LG=$ARGV[0];
my $EAGLES=$ARGV[1];

my $nt;
my $nokS;
my $nokL;

while (<STDIN>) {
  chomp $_;
  my @line=split("[ \n\t]",$_);

  if ($#line>0) {
    $nt++;
    if (ShortTag($line[2],$LG,$EAGLES) eq ShortTag($line[5],$LG,$EAGLES)) {$nokS++;}
    if ($line[2] eq $line[5]) {$nokL++;}
  }
}

print "Accuracy. short tag:".(100*$nokS/$nt)."  long tag:".(100*$nokL/$nt)."\n";


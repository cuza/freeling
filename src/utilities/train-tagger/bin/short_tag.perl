
#################  return short tag  ###########

sub ShortTag {
    my ($tag,$lang,$eagles) = @_;
    my ($tall);
  
    if (! $eagles) {return $tag;}  # not using parole, return tag untouched
    
    if ($tag =~ /^F/ || $tag eq "OUT_OF_BOUNDS") {$tall = substr ($tag,0);}
    elsif ($tag =~ /^V/ or ($tag =~ /^[VDN]/ and $lang eq "ru")) {$tall = substr ($tag,0,3);}
    else {$tall = substr ($tag,0,2);}

    return $tall;
}

1;


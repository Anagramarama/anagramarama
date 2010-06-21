# load the unix system dictionary and eliminate all words that have apostrophes
# or are proper names and any that are less than 3 chars or more than 7 chars.
# compare the words against the scrabble lexicon file for validity and
# discard those not present in that file (check for an english one).


# load the scrabble words
proc load_lexicon {filename} {
    variable lexicon
    set f [open $filename r]
    gets $f junk ;# discard first line which is the count of words.
    while {[gets $f word] != -1} {
        set lexicon([string tolower $word]) 1
    }
    close $f
    return
}

proc main {filename} {
    variable lexicon
    set t [time {load_lexicon TWL06.txt}]
    puts stderr "lexicon loaded in [expr {[lindex $t 0]/1000}] ms"

    # Read the unix dictionary file and prune out the undesirables.
    set f [open $filename r]
    fconfigure $f -encoding utf-8
    while {[set len [gets $f word]] != -1} {
        if {$len > 7 || $len < 3} continue
        if {[string is upper [string index $word 0]]} continue
        if {[string first "'" $word] != -1} continue
        if {$word ne [string tolower $word]} continue
        if {[encoding convertto ascii $word] ne $word} continue
        if {[info exists lexicon($word)]} {
            puts $word
        } else {
            puts stderr "skip $word"
        }
    }
    close $f
}

if {!$tcl_interactive} {
    set r  [catch [linsert $argv 0 main] err]
    if {$r} {puts $errorInfo}
    exit $r
}

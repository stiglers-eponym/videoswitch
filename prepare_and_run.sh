#!/bin/bash

if [ "$1" == "-h" ] || [ "$1" == "--help" ]
then
    cat <<EOF
Prepare and run videoswitch.

Options required for preparation must be set as environment variables:
    WIDTH  = 1920
    HEIGHT = 1080
    WORDLIST_SOURCE = "wordlist_init.txt"
    WORDLIST = "/tmp/wordlist.txt"
    PICTURE = "/tmp/wordcloud.png"
    MASK_SOURCE = "mask.svg"
    MASK = "/tmp/mask.png"
    PROGRAM = "gen_wordcloud.py"

Other options are passed directly to videoswitch:
  --playlist <file>              playist json file path
  --max_weight <int>             maximum weight of word added to word list
  --fade_in_duration <int>       duration of video fading in, in ms
  --fade_out_duration <int>      duration of video fading out, in ms
  --image_change_duration <int>  duration of image change transition, in ms
  --font_size <int>              font size in control window
EOF
    exit 0
fi

set -ex

# Define default values for arguments. These can be set as environment variables.
: ${WIDTH:=1920}
: ${HEIGHT:=1080}
: ${WORDLIST_SOURCE:="wordlist_init.txt"}
: ${WORDLIST:="/tmp/wordlist.txt"}
: ${PICTURE:="/tmp/wordcloud.png"}
: ${MASK_SOURCE:="mask.svg"}
: ${MASK:="/tmp/mask.png"}
: ${PROGRAM:="gen_wordcloud.py"}


[ -e "$WORDLIST" ] || cp "$WORDLIST_SOURCE" "$WORDLIST"
inkscape -o "$MASK" -w "$WIDTH" -h "$HEIGHT" -y 255 "$MASK_SOURCE" \
    || inkscape --export-png="$MASK" -w "$WIDTH" -h "$HEIGHT" -y 255 "$MASK_SOURCE" # old syntax
python3 "$PROGRAM" mask="$MASK" input="$WORDLIST" output="$PICTURE"
./videoswitch -W "$WIDTH" -H "$HEIGHT" --mask "$MASK" --wordlist "$WORDLIST" --image "$PICTURE" --program "$PROGRAM" $@

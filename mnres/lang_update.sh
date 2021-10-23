#!/bin/bash

set -e

if [[ ! -f ./lang_update.sh ]]; then
  echo please run at /mnres dir
  exit 1
fi

xgettext  \
  --from-code=UTF-8  \
  --keyword="_"  \
  --keyword="_r"  \
  --omit-header  \
  --output=temp.pot  \
  --sort-by-file  \
  $(find ../mnian/ -name "*.cc")

msgmerge  \
  -U  \
  --backup=none  \
  lang_english.po  \
  temp.pot

rm temp.pot

#!/bin/bash

if [[ ! -f ./lang_update.sh ]]; then
  echo please run at /mnres dir
  exit 1
fi

xgettext  \
  --from-code=UTF-8  \
  --join-existing  \
  --keyword="_"  \
  --omit-header  \
  --output=lang_english.po  \
  --sort-by-file  \
  $(find ../mnian/ -name "*.cc")

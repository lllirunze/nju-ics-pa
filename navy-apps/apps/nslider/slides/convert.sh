#!/bin/bash

set -e
cd "$(dirname "$0")"

# Keep the source page's aspect ratio and place it on a 4:3 canvas.  This
# avoids stretching a non-4:3 PDF when generating NSlider's 400x300 bitmaps.
rm -f -- slides-*.bmp
gs -q -dSAFER -dBATCH -dNOPAUSE -sDEVICE=bmp32b -r72 -g400x300 \
  -dPDFFitPage -sOutputFile=slides-%d.bmp slides.pdf

# Ghostscript numbers pages from 1, while NSlider expects pages from 0.
page=1
i=0
while [ -f "slides-$page.bmp" ]; do
  mv -- "slides-$page.bmp" "slides-$i.bmp.tmp"
  page=$((page + 1))
  i=$((i + 1))
done
for slide in slides-*.bmp.tmp; do
  mv -- "$slide" "${slide%.tmp}"
done

mkdir -p "$NAVY_HOME/fsimg/share/slides/"
rm -f -- "$NAVY_HOME"/fsimg/share/slides/*.bmp
mv -- slides-*.bmp "$NAVY_HOME/fsimg/share/slides/"

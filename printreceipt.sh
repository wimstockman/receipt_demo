#!/bin/bash
# Created by Wim Stockman wim@thinkerwim.org on 2022-12-14 

## Purpose of Script
# Generate a perfect length receipt and print that receipt with custom paper lengt on a Star TSP Thermal printer

## Explaination
# Groff is called a first time without output to get the length of the ticket which we store in size variable
# Second time we call groff with the right page length for the PostScript device and generate the postscript file
# Third time we do the same but to generate a pdf file. This is just for example.
# In real life you choose one of the two ps or pdf
# next we add some extra papermargin for the printer I added 5mm 
# and finally we send the file to the printer
 
size=$(groff -M"./" -mreceipt -P-p66.5c,7c -z receipt.groff 2>&1)
size=$(echo "scale=2;$size / 28346 + 0.5" | bc)
groff -M"./" -mreceipt -P-p${size}c,6.8c receipt.groff > receipt.ps 2>/dev/null
groff -M"./" -mreceipt -Tpdf -P-p${size}c,6.8c receipt.groff > receipt.pdf 2>/dev/null
printsize=$(echo "scale=1;$size * 10 + 5 " | bc)
printsize=$(echo $(printf %.0f $printsize))
lp -s -d STAR -o media=custom.72x${printsize}mm receipt.pdf


## Files used for this little tool
# receipt.groff
# receipt.tmac
# receipt.ps
# receipt.pdf

## Dependencies
# bash
# bc
# cups
# echo
# groff

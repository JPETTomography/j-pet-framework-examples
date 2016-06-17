#!/bin/bash
#if the first argument is provided it is treated as the output path
#e.g. download_data.sh /blabla/path
#will write the output to /blabla/path 


#analysis example
DIR_AE=AnalysisExample

#scope reader example
DIR_SRE=ScopeLoaderExample

#scope reader example
DIR_LBE=LargeBarrelExample

HTTP_PATH="http://koza.if.uj.edu.pl/framework/Examples"
WGET_OUTPUT="./"
# -r  means recursive, 
# --cut-dirs=1 ignore given level of directories (e.g. remove framework from path) 
# -nH  Disable generation of host-prefixed directories (so save only what is inside Examples and not full path)
# -np no parents
# --reject="index.html*" - do not store all index.html files
# -e robots=off - remove some robot generated files
declare -a WGET_FLAGS=(-r -nH -np --cut-dirs=2 --reject="index.html*" -e robots=off)

#if there is an extra argument treat it as the output path 
#if the argument is empty set some default paths
if [ -z $1 ]; then
  WGET_OUTPUT=.
else
  WGET_OUTPUT=$1
fi


WGET_DIR=${DIR_AE}
WGET_INPUT=${HTTP_PATH}/${DIR_AE}
#downloading test data via wget
wget "${WGET_INPUT}" "${WGET_FLAGS[@]}" -P "${WGET_OUTPUT}" 

WGET_DIR=${DIR_SRE}
WGET_INPUT=${HTTP_PATH}/${DIR_SRE}
#downloading test data via wget
get "${WGET_INPUT}" "${WGET_FLAGS[@]}" -P "${WGET_OUTPUT}" 

WGET_DIR=${DIR_LBE}
WGET_INPUT=${HTTP_PATH}/${DIR_LBE}
#downloading test data via wget
get "${WGET_INPUT}" "${WGET_FLAGS[@]}" -P "${WGET_OUTPUT}" 

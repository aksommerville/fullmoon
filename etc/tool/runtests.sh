#!/bin/bash

PASSC=0
FAILC=0
SKIPC=0

log_pass() { echo -e "\x1b[32mPASS\x1b[0m $1" ; }
log_fail() { echo -e "\x1b[31mFAIL\x1b[0m $1" ; }
log_skip() { echo -e "\x1b[37mSKIP\x1b[0m $1" ; }
log_detail() { echo "$1" ; }
log_loose() { echo "$1" ; }

runtest() {
  while IFS= read INPUT ; do
    read INTRODUCER KEYWORD ARGS <<<"$INPUT"
    if [ "$INTRODUCER" = FMN_TEST ] ; then
      case "$KEYWORD" in
        PASS) PASSC=$((PASSC+1)) ; log_pass "$ARGS" ;;
        FAIL) FAILC=$((FAILC+1)) ; log_fail "$ARGS" ;;
        SKIP) SKIPC=$((SKIPC+1)) ; log_skip "$ARGS" ;;
        DETAIL) log_detail "$ARGS" ;;
        *) log_loose "$INPUT" ;;
      esac
    else
      log_loose "$INPUT"
    fi
  done < <( $1 2>&1 || echo "FMN_TEST FAIL $1" )
}

for EXE in $* ; do
  runtest $EXE
done

if [ "$FAILC" -gt 0 ] ; then
  echo -e "\x1b[41m    \x1b[0m $FAILC fail, $PASSC pass, $SKIPC skip"
  exit 1
elif [ "$PASSC" -gt 0 ] ; then
  echo -e "\x1b[42m    \x1b[0m $FAILC fail, $PASSC pass, $SKIPC skip"
else
  echo -e "\x1b[47m    \x1b[0m $FAILC fail, $PASSC pass, $SKIPC skip"
fi

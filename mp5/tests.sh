#!bin/bash


leakcheck() {
  rm -f .leaks
  valgrind --trace-children=yes --leak-check=full -q --log-file=.leaks "$@" >/dev/null 2>/dev/null
  [ -e .leaks ] && stat -c %s .leaks || echo 0
  rm .leaks
}

score=0
mult=1.0

_showgrade() { 
  echo "SCORE: "$score" / 100"
  echo "SCORE MULTIPLIER: "${mult}
}
trap _showgrade EXIT;

function score_case {
  pts=$1
  shift
  which="$1"
  shift
  if [ "$#" -gt 0 ]; then timeout="$1"; else timeout="1.0"; fi
  
  if timeout $timeout ./tester --out /dev/null -w NoTests "$which"
  then
    score=$((score+pts))
    echo "✅ $which"
    return 0
  else
    echo "❌ $which"
    return 1
  fi
}

function score_pytest_case {
  pts=$1
  shift
  which="$1"
  shift
  if [ "$#" -gt 0 ]; then timeout="$1"; else timeout="5.0"; fi
  
  if timeout $timeout python3 -m pytest -k "$which" -qq >/dev/null
  then
    score=$((score+pts))
    echo "✅ $which"
    return 0
  else
    echo "❌ $which"
    return 1
  fi
}


make tester

score_case 3 'httprequest_parse_headers - Request Line' 2.0
score_case 3 'httprequest_parse_headers - No Header'
score_case 3 'httprequest_parse_headers - First Header'
score_case 3 'httprequest_parse_headers - Two Headers'
score_case 3 'httprequest_parse_headers - Content-Length Header'
score_case 5 'httprequest_parse_headers - N Headers'
score_case 2 'httprequest_read - Request Line'
score_case 2 'httprequest_read - First Header'
score_case 2 'httprequest_read - Two Headers'
score_case 2 'httprequest_read - Content-Length Header'
score_case 2 'httprequest_read - Invalid Content-length'
score_case 2 'httprequest_read - No Content-Length results in a NULL payload'
score_case 2 'httprequest_read - ASCII Payload Data'
score_case 4 'httprequest_read - Payload Binary Payload'
score_case 4 'httprequest_read - Arbitrary Small Binary Payload'
score_case 4 'httprequest_read - Arbitrary Long Binary Payload'
score_case 5 'httprequest_read - Binary Payload with HTTP Payload Delimiter'
score_case 2 'httprequest_read - Small Payload (50 B)'
score_case 3 'httprequest_read - Medium Payload (50 KiB)'
score_case 4 'httprequest_read - Large Payload (5 MiB)' 2.0

score_pytest_case 5 'test_server[-200-None]' 5
score_pytest_case 10 'test_server[microbe.html-200-text_html]' 5
score_pytest_case 10 'test_server[340.png-200-image_png]' 5
score_pytest_case 15 'test_server[not-found.html-404-None]' 5

vg=$(leakcheck ./tester "[part=2]" || echo 0)
if [ "$vg" -gt 1 ]
then
  echo "❌ valgrind"
  mult=0.75
else
  echo "✅ valgrind"
fi

true


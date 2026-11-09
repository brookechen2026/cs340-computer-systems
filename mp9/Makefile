.PHONY: test run view background stop

run:
	python3 mazegen.py

test:
	python3 -m pytest

view:
	python3 mazeview.py &
	python3 mazegen.py

background:
	nohup python3 mazegen.py </dev/null &>/dev/null &

stop:
	killall -SIGINT python3

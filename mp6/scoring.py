from subprocess import run
import re
import glob

res = run(['python3','-m', 'pytest', 'test_coding_rules.py', '-q','-r','A'], capture_output=True)

if res.stdout.decode('utf-8').count('PASSED') != 3:
  print(res.stdout.decode('utf-8'))
  print('SCORE: 0 / 100')
  quit()



res = run(['python3','-m', 'pytest', '-q','-r','A']+sorted([_ for _ in glob.glob('test_*py') if _ != 'test_coding_rules.py']), capture_output=True)
k = res.stdout.decode('utf-8')
k = k[k.rfind('short test summary info'):]
k = k[k.find('\n'):]

weights = {}

score = 0
for [f,t] in re.findall(r'\nPASSED ([^\n]*).py::([a-zA-Z0-9_]+)', k):
  default = 2
  if 'part_1' in f or 'part_4' in f: default = 4
  score += weights.get(t,default)

print(k)
print('SCORE:',score,'/',100)

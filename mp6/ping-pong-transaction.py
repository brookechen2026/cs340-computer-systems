import sys
import threading
import random
from wallet import Wallet

def ping(wallet):
  for i in range(3000):
    wallet.transaction(ping=+1, pong=-1)
    sys.stderr.write('↑')

def pong(wallet):
  for i in range(3000):
    wallet.transaction(ping=-1, pong=+1)
    sys.stderr.write('↓')

if __name__ == '__main__':
  print("ping-pong with transactions");
  wallet = Wallet()
  if random.randrange(2) == 0:
    wallet.change('pong', +1)
    print('Should start with ↑')
  else:
    wallet.change('ping', +1)
    print('Should start with ↓')
  t1 = threading.Thread(target=ping, args=[wallet])
  t2 = threading.Thread(target=pong, args=[wallet])
  t1.start()
  t2.start()
  t1.join()
  t2.join()
  print(file=sys.stderr)
  

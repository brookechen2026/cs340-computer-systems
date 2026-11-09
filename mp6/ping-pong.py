import sys
import threading
from wallet import Wallet

def ping(wallet):
  for i in range(3000):
    wallet.change('ping',+1)
    wallet.change('pong',-1)
    sys.stderr.write('↑')

def pong(wallet):
  for i in range(3000):
    wallet.change('ping',-1)
    wallet.change('pong',+1)
    sys.stderr.write('↓')

if __name__ == '__main__':
  print("ping-pong");
  wallet = Wallet()
  t1 = threading.Thread(target=ping, args=[wallet])
  t2 = threading.Thread(target=pong, args=[wallet])
  t1.start()
  t2.start()
  t1.join()
  t2.join()
  print(file=sys.stderr)
  

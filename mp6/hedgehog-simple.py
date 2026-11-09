import sys
import threading
from wallet import Wallet
import time
import random

def test_add_hedghog_food(wallet):
  for i in range(500):
    wallet.change('hedgehog-food',+1)
    sys.stderr.write('🐛')
    if random.randrange(6) == 0: time.sleep(0.000001)
    

def test_add_hedghog(wallet):
  for i in range(100):
    wallet.change('hedgehog-food',-3)
    wallet.change('hedgehog',+1)
    sys.stderr.write('🦔')

if __name__ == '__main__':
  print("Running Test: Test hedgehog food and hedgehog");
  wallet = Wallet()
  t1 = threading.Thread(target=test_add_hedghog, args=[wallet])
  t2 = threading.Thread(target=test_add_hedghog_food, args=[wallet])
  t1.start()
  t2.start()
  t1.join()
  t2.join()
  print(file=sys.stderr)
  print('Remaining Hedgehog Food 🐛:', wallet.get('hedgehog-food'))
  print('Hedgehogs 🦔:', wallet.get('hedgehog'))
  

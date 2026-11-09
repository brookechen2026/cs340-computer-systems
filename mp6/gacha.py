import sys
import threading
from wallet import Wallet
import time
import random

def generate_primogen(wallet):
  total = 0
  while total < 14400:
    r = random.uniform(0,1)
    if r < 0.25: amount = 10
    elif r < 0.75: amount = 20
    else: amount = 50
    wallet.change('primogen', amount)
    total += amount
    sys.stderr.write('✨×{} '.format(amount))
    time.sleep(0.000001)
    

def fate(wallet):
  for i in range(90):
    wallet.change('primogen',-160)
    wallet.change('fate',+1)
    sys.stderr.write('💫 ')

def wish(wallet):
  for i in range(90):
    wallet.transaction(fate=-1, wish=+1)
    sys.stderr.write('🌠 ')

pity_4s = 0
pity_5s = 0

def gacha(wallet):
  global pity_4s, pity_5s
  for i in range(90):
    wallet.change('wish',-1)
    
    r = random.uniform(0,1)
    if r < 0.006: result = 5
    elif r < 0.057: result = 4
    else: result = 3
    
    if result < 4 and pity_4s >= 9: result = 4
    if result < 5 and pity_5s >= 89: result = 5
    
    if result != 4: pity_4s += 1
    if result != 5: pity_5s += 1
    
    if result == 5:
      pity_5s = 0
      sys.stderr.write('🌟🌟🌟🌟🌟\n')
      wallet.change('5*',1)
    elif result == 4:
      pity_4s = 0
      sys.stderr.write('⭐⭐⭐⭐\n')
      wallet.change('4*',1)
    else:
      sys.stderr.write('★★★\n')
      wallet.change('3*',1)
    
  

if __name__ == '__main__':
  print("gacha simulation")
  wallet = Wallet()
  t1 = threading.Thread(target=generate_primogen, args=[wallet])
  t2 = threading.Thread(target=fate, args=[wallet])
  t3 = threading.Thread(target=wish, args=[wallet])
  t4 = threading.Thread(target=gacha, args=[wallet])
  for t in t1,t2,t3,t4: t.start()
  for t in t1,t2,t3,t4: t.join()
  print(file=sys.stderr)
  five = wallet.get('5*')
  print('🌟🌟🌟🌟🌟 found:', five)
  print('⭐⭐⭐⭐ found:', wallet.get('4*'))
  print('★★★ found:', wallet.get('3*'))
  print()
  if five == 0: print('!!: Yikes, your wallet must not be correct.')
  elif five == 1: print('OK, and average luck.')
  elif five == 2: print('WOW, you had exceptional luck.')
  elif five == 3: print('AMAZING, the luck to get this is INSANE!')
  else: print('...I think your wallet is broken. ☹')

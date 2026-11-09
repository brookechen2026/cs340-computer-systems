import sys
import threading
from wallet import Wallet
import time
import random





def job_clover_patch(wallet):
  for i in range(100):
    wallet.change("clover", 1); sys.stderr.write("☘️");
    wallet.change("clover", 1); sys.stderr.write("☘️");
    wallet.change("clover", 1); sys.stderr.write("☘️");
    wallet.change("four_leaf_clover", 1); sys.stderr.write("🍀");
    wallet.change("clover", 1); sys.stderr.write("☘️");
    wallet.change("clover", 1); sys.stderr.write("☘️");
    wallet.change("clover", 1); sys.stderr.write("☘️");
    wallet.change("clover", 1); sys.stderr.write("☘️");
    if random.randrange(10) == 0:
      wallet.change("four_leaf_clover", 1); sys.stderr.write("🍀");
    wallet.change("clover", 1); sys.stderr.write("☘️");
    wallet.change("clover", 1); sys.stderr.write("☘️");
    wallet.change("clover", 1); sys.stderr.write("☘️");
    wallet.change("clover", 1); sys.stderr.write("☘️");
    time.sleep(random.uniform(0,0.0001))


def job_orchard(wallet):
  for i in range(110):
    wallet.change("green_apple", 1);
    sys.stderr.write("🍏");
    time.sleep(random.uniform(0,0.0001))


def job_workshop(wallet):
  for i in range(520):
    wallet.change("tools", 1);
    sys.stderr.write("🧰");

    if (i % 5 == 0):
      wallet.change("gem", 1);
      sys.stderr.write("💎");
    time.sleep(random.uniform(0,0.0001))


def job_dna(wallet):
  for i in range(1750):
    wallet.change("dna", 1);
    sys.stderr.write("🧬");
    time.sleep(random.uniform(0,0.0001))


def job_research_green(wallet):
  # 📗 requires 1x🍏 1x🍀 10x☘️ 5x🧬
  for i in range(100):
    wallet.transaction(green_apple=-1, four_leaf_clover=-1, clover=-10, dna=-5, green_book=1);
    sys.stderr.write("📗");
    time.sleep(random.uniform(0,0.0001))

def job_research_blue(wallet):
  # 📘 requires 10x🧬 1x💎
  for i in range(100):
    wallet.transaction(dna=-10, gem=-1, blue_book=1);
    sys.stderr.write("📘");
    time.sleep(random.uniform(0,0.0001))

def job_research_orange(wallet):
  # 📙 requires 5x🧰 2x🧬
  for i in range(100):
    wallet.change("tools", -5);
    wallet.change("dna", -2);
    wallet.change("orange_book", 1);
    sys.stderr.write("📙");
    time.sleep(random.uniform(0,0.0001))

def job_combine_research(wallet):
  # 📚 requires 1x📗, 1x📘, 1x📙
  for i in range(100):
    wallet.change("orange_book", -1);
    wallet.change("blue_book", -1);
    wallet.change("green_book", -1);
    wallet.change("books", 1);
    sys.stderr.write("📚");
    time.sleep(random.uniform(0,0.0001))

def job_graduation(wallet):
  # 🎓 requires 100x 📚 
  wallet.transaction(books=-100, degree=1);
  sys.stderr.write("🎓");
  time.sleep(random.uniform(0,0.0001))



if __name__ == '__main__':
  sys.stderr.write("Resources generated: ");
  wallet = Wallet()
  tids = [threading.Thread(target=globals()[n], args=[wallet]) for n in dir() if n.startswith('job_')]
  for t in tids: t.start()
  for t in tids: t.join()
  print(file=sys.stderr)

  if wallet.get('degree') == 1:
    print("Your wallet contains a degree! 🎓")
    print("- Extra 🧬:", wallet.get('dna'))
    print("- Extra 🍀:", wallet.get('four_leaf_clover'))
  else:
    print("Yikes -- your wallet may not have that degree yet... ☹")

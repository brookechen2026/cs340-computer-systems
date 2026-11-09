
def test_atomic():
    assert not run_one_test(case_atomic)

def case_atomic():
    from wallet import Wallet
    from threading import Thread
    import time
    w = Wallet()
    t1 = Thread(target=lambda w: w.transaction(a=-1,b=-1,c=-1,d=3), args=(w,))
    t1.start()
    time.sleep(0.001)
    w.change('a', 1)
    w.change('b', 1)
    time.sleep(0.001)
    w.change('a', -1)
    w.change('c', 1)
    time.sleep(0.001)
    w.change('b', -1)
    w.change('a', 1)
    time.sleep(0.001)
    if not t1.is_alive():
        return 'Transaction ended early'
    w.change('b', 1)
    t1.join()
    if w.get('d') != 3:
        return 'Transaction gave wrong final result'

def test_nothing():
    assert not run_one_test(case_nothing)

def case_nothing():
    from wallet import Wallet
    w = Wallet()
    w.transaction()

def test_just_add():
    assert not run_one_test(case_just_add)

def case_just_add():
    from wallet import Wallet
    w = Wallet()
    w.transaction(value=10)
    if w.get('value') != 10: return 'Single positive value transaction failed'
    w.transaction(value=15)
    if w.get('value') != 25: return 'Sequence of single positive value transactions failed'

def test_blocking():
    assert not run_one_test(case_blocking, should_block=True)

def case_blocking():
    from wallet import Wallet
    w = Wallet()
    w.change('deficit',5)
    w.transaction(deficit=-7)
    assert not "unreachable"


def test_retval():
    assert not run_one_test(case_retval)

def case_retval():
    from wallet import Wallet
    w = Wallet()
    w.change('src', 50)
    w.change('d2', 5)
    if w.transaction(src=-5, d1=2, d2=1, d3=2) != {'src':45, 'd1':2, 'd2':6, 'd3':2}:
        return 'wrong transaction return value'


def test_scatter():
    assert not run_one_test(case_scatter)

def case_scatter():
    from wallet import Wallet
    w = Wallet()
    w.change('src', 50)
    w.transaction(src=-5, d1=2, d2=1, d3=2)
    w.transaction(src=-5, d1=2, d3=1, d5=2)
    w.transaction(src=-5, d2=2, d3=1, d5=2)
    w.transaction(src=-10, d1=1, d2=1, d3=1, d4=1, d5=1, d6=1, d7=1, d8=1, d9=1, d10=1)
    if w.get('src') != 25: return 'Wrong src value'
    if w.get('d1') != 5: return 'Wrong d1 value'
    if w.get('d2') != 4: return 'Wrong d2 value'
    if w.get('d3') != 5: return 'Wrong d3 value'
    if w.get('d4') != 1: return 'Wrong d4 value'
    if w.get('d5') != 5: return 'Wrong d5 value'
    if w.get('d6') != 1: return 'Wrong d6 value'
    if w.get('d7') != 1: return 'Wrong d7 value'
    if w.get('d8') != 1: return 'Wrong d8 value'
    if w.get('d9') != 1: return 'Wrong d9 value'
    if w.get('d10') != 1: return 'Wrong d10 value'

def test_gather():
    assert not run_one_test(case_gather)

def case_gather():
    from wallet import Wallet
    w = Wallet()
    w.try_change('bread',12)
    w.try_change('pb',16)
    w.try_change('mustard',4)
    w.try_change('sardines',20)
    w.try_change('raisins',200)
    w.try_change('cheese',16)
   
    w.transaction(bread=-2, pb=-1, mustard=-1, sardines=-8, raisins=-30, cheese=-2, sandwich=44)
    if w.get('bread') != 10: return 'did not use up bread'
    if w.get('raisins') != 170: return 'did not use up raisins'
    if w.get('sandwich') != 44: return 'did not create sandwich'

    w.transaction(bread=-2, pb=-1, mustard=-1, sardines=-8, raisins=-30, cheese=-2, sandwich=44)
    if w.get('bread') != 8: return 'did not continue using up bread'
    if w.get('raisins') != 140: return 'did not continue using up raisins'
    if w.get('sandwich') != 88: return 'did not continue creating sandwiches'

def test_chain():
    assert not run_one_test(case_chain)

def case_chain():
    from wallet import Wallet
    from threading import Thread
    import time
    w = Wallet()
    
    ts = [
        Thread(target=lambda w: w.transaction(a=1,b=-1), args=(w,)),
        Thread(target=lambda w: w.transaction(b=1,c=-1), args=(w,)),
        Thread(target=lambda w: w.transaction(c=1,d=-1), args=(w,)),
        Thread(target=lambda w: w.transaction(d=1,e=-1), args=(w,)),
        Thread(target=lambda w: w.transaction(e=1,f=-1), args=(w,)),
        Thread(target=lambda w: w.transaction(f=1,g=-1), args=(w,)),
        Thread(target=lambda w: w.transaction(g=1,h=-1), args=(w,)),
        Thread(target=lambda w: w.transaction(h=1,i=-1), args=(w,)),
        Thread(target=lambda w: w.transaction(i=1,a=-1), args=(w,)),
    ]
    for t in ts: t.start()
    time.sleep(0.001)
    for t in ts:
        if not t.is_alive(): return 'Ring of transactions should have blocked'
    w.try_change('c',1)
    for t in ts: t.join()
    if w.get('c') != 1: return 'Ring of transactions failed to update resources correctly'
    if w.get('a') != 0: return 'Ring of transactions failed to update resources correctly'
    if w.get('h') != 0: return 'Ring of transactions failed to update resources correctly'


def wrapper(q, fn, args):
    try:
        q.put(fn(*args))
    except BaseException as ex:
        q.put(ex)


def run_one_test(fn, should_block=False, args=(), timeout=0.2):
    from multiprocessing import Queue, Process
    q = Queue()
    p = Process(target=wrapper, args=(q, fn, args))
    p.start()
    p.join(timeout)
    msg = [q.get()] if not q.empty() else []
    if should_block and not p.is_alive():
        msg.append('Expected to block, but did not')
    elif not should_block and p.is_alive():
        msg.append('Blocked, but should have finished in under {} seconds'.format(timeout))
    if p.is_alive(): p.kill()
    return msg[0] if msg else None


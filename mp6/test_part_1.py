def case_initially_empty():
    from wallet import Wallet
    import random
    w = Wallet()
    rstring = chr(random.randrange(0x20, 0x7f)) + chr(random.randrange(0x20, 0x7f)) + chr(random.randrange(0x20, 0x7f))
    for t in ('hedgehog','star','nothing',rstring):
        if w.get(t) != 0: return 'new wallet resource '+repr(t)+' nonzero'
    return None

def test_initially_empty():
    assert not run_one_test(case_initially_empty)


def case_two_wallets_empty():
    from wallet import Wallet
    import random
    w = Wallet()
    rstring = chr(random.randrange(0x20, 0x7f)) + chr(random.randrange(0x20, 0x7f)) + chr(random.randrange(0x20, 0x7f))
    for t in ('hedgehog','star','nothing',rstring):
        w.change(t, 1)

    w = Wallet()
    for t in ('hedgehog','star','nothing',rstring):
        if w.get(t) != 0: return 'second wallet resource'+repr(t)+' nonzero after first wallet changed it'

    return None

def test_two_wallets_empty():
    assert not run_one_test(case_two_wallets_empty)

def case_change_changes():
    from wallet import Wallet
    w = Wallet()
    if w.get('demo') != 0: return 'resource started nonzero'
    w.change('demo',5)
    if w.get('demo') != 5: return 'first change lost by second change'

    return None

def test_change_changes():
    assert not run_one_test(case_change_changes)

def case_change_return():
    from wallet import Wallet
    w = Wallet()
    if w.get('demo') != 0: return 'resource started nonzero'
    if w.change('demo',5) != 5: return 'first change did not return new value'
    if w.get('demo') != 5: return 'first change not reflected in subsequent get'
    if w.change('demo',3) != 8: return 'second change did not return new value'
    if w.get('demo') != 8: return 'second change not reflected in subsequent get'

    return None

def test_change_return():
    assert not run_one_test(case_change_return)

def case_negative_changes():
    from wallet import Wallet
    w = Wallet()
    w.change('whatnot', 5)
    w.change('whatnot', -2)
    w.change('whatnot', 15)
    if w.change('whatnot', -8) != 10: return 'negative change did not return new value'
    if w.change('whatnot', -10) != 0: return 'negative change did not return new value'
    if w.get('whatnot') != 0: return 'chain of positive and negative changes failed to produce correct value'

    return None

def test_negative_changes():
    assert not run_one_test(case_negative_changes)

def case_many_changes():
    from wallet import Wallet
    w1 = Wallet()
    w2 = Wallet()
    for i in range(100):
        w1.change('thing'+str(i), 1+i)
        w2.change('thing'+str(i), 100-i)
    for i in range(100):
        w1.change('thing'+str(i), -1)
        w2.change('thing'+str(i), 1)
    for i in range(100):
        w1.change('thing100', 2)
        w1.change('thing100', -1)
        w2.change('thing100', 2000)
        w2.change('thing100', -1990)
    
    for i in range(100):
        if w1.get('thing'+str(i)) != i:
            return 'wallet 1 thing'+str(i)+' should be '+str(i)+' not '+str(w1.get('thing'+str(i)))
        if w2.get('thing'+str(i)) != 101-i:
            return 'wallet 2 thing'+str(i)+' should be '+str(101-i)+' not '+str(w2.get('thing'+str(i)))
    if w1.get('thing100') != 100:
        return 'wallet 1 thing100 should be 100 not '+str(w1.get('thing100'))
    if w2.get('thing100') != 1000:
        return 'wallet 2 thing100 should be 1000 not '+str(w2.get('thing100'))

    return None

def test_many_changes():
    assert not run_one_test(case_many_changes)



def wrapper(q, fn, args):
    try:
        q.put(fn(*args))
    except BaseException as ex:
        q.put(ex)


def run_one_test(fn, should_block=False, args=(), timeout=0.35):
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










def test_empty():
    assert not run_test_with_server(case_empty)

def case_empty(port):
    from socket import socket, AF_INET, SOCK_STREAM
    with socket(AF_INET, SOCK_STREAM) as s:
        s.connect(('localhost', port))
        s.send(b'GET random_name\n')
        p = s.recv(1024)
        if p.strip() != b'0':
            return 'GET with unset value failed; gave '+repr(p)

def test_one_resource():
    assert not run_test_with_server(case_one_resource)

def case_one_resource(port):
    from socket import socket, AF_INET, SOCK_STREAM
    with socket(AF_INET, SOCK_STREAM) as s:
        s.connect(('localhost', port))

        s.send(b'GET nonsense\n')
        p = s.recv(1024)
        if p.strip() != b'0':
            return 'GET with unset value failed; gave '+repr(p)

        s.send(b'MOD nonsense 8\n')
        if s.recv(1024).strip() != b'8': return 'MOD had wrong return value'

        s.send(b'MOD nonsense -3\n')
        if s.recv(1024).strip() != b'5': return 'Second MOD had wrong return value'

        s.send(b'GET nonsense\n')
        if s.recv(1024).strip() != b'5': return 'GET after MOD had wrong return value'

def test_blocking():
    assert not run_test_with_server(case_blocking, should_block=True)

def case_blocking(port):
    from socket import socket, AF_INET, SOCK_STREAM
    with socket(AF_INET, SOCK_STREAM) as s:
        s.connect(('localhost', port))
        s.send(b'MOD something 3\n')
        s.recv(1024)
        s.send(b'MOD something -5\n')
        s.recv(1024)
        return 'Blocking MOD should not return'

def test_nonblocking():
    assert not run_test_with_server(case_nonblocking)

def case_nonblocking(port):
    from socket import socket, AF_INET, SOCK_STREAM
    with socket(AF_INET, SOCK_STREAM) as s:
        s.connect(('localhost', port))
        s.send(b'MOD money 5\n')
        s.recv(1024)
        s.send(b'TRY money -6\n')
        p = s.recv(1024)
        if p.strip() != b'False':
            return 'TRY that fails should send back False, not '+repr(p)

def test_wait_wakeup():
    assert not run_test_with_server(case_wait_wakeup, timeout=0.5)

def case_wait_wakeup(port):
    from threading import Thread
    from socket import socket, AF_INET, SOCK_STREAM
    import time
    messages = []
    
    def waiter(port):
        with socket(AF_INET, SOCK_STREAM) as s:
            s.connect(('localhost', port))
            s.send(b'MOD cash -4\n')
            s.recv(1024)
            s.send(b'MOD cash 6\n')
            s.recv(1024)
            s.send(b'MOD cash -8\n')
            p = s.recv(1024)
            if p.strip() != b'2':
                messages.append('final value incorrect')
    def unwaiter(port):
        with socket(AF_INET, SOCK_STREAM) as s:
            s.connect(('localhost', port))
            s.send(b'MOD cash 4\n')
            s.recv(1024)
            s.send(b'MOD cash -4\n')
            s.recv(1024)
            s.send(b'MOD cash 8\n')
            s.recv(1024)
            
    tw = Thread(target=waiter, args=(port,))
    tu = Thread(target=unwaiter, args=(port,))
    tw.start()
    time.sleep(0.1)
    if not tw.is_alive(): return 'Initial thread should have blocked'
    tu.start()
    tu.join()
    tw.join()
    if len(messages): return messages[0]

def test_many_connections():
    assert not run_test_with_server(case_many_connections, timeout=2)

def case_many_connections(port):
    from threading import Thread
    from socket import socket, AF_INET, SOCK_STREAM
    import time
    
    def take_sender(src, dst, num):
        def f(port):
            with socket(AF_INET, SOCK_STREAM) as s:
                s.connect(('localhost', port))
                s.send(('MOD '+src+' '+str(-num)+'\n').encode('utf-8'))
                s.recv(1024)
                s.send(('MOD '+dst+' '+str(num)+'\n').encode('utf-8'))
                s.recv(1024)
        return f
    ts = [Thread(target=take_sender('cash',str(i),i), args=(port,)) for i in range(1,101)]
    for t in ts: t.start()
    
    with socket(AF_INET, SOCK_STREAM) as s:
        s.connect(('localhost', port))
        for i in range(505):
            s.send(b'MOD cash 10\n')
            s.recv(1024)

    for t in ts: t.join()

    with socket(AF_INET, SOCK_STREAM) as s:
        s.connect(('localhost', port))
        for i in range(505):
            s.send(b'GET cash\n')
            p = s.recv(1024) 
            if p.strip() != b'0':
                return 'failed to handle 100 sockets correctly '+repr(p)

def test_transaction():
    assert not run_test_with_server(case_transaction)

def case_transaction(port):
    from socket import socket, AF_INET, SOCK_STREAM
    with socket(AF_INET, SOCK_STREAM) as s:
        s.connect(('localhost', port))
        s.send(b'MOD src 50\n')
        s.recv(1024)
        s.send(b'MOD d2 5\n')
        s.recv(1024)
        s.send(b'TRAN src -5 d1 2 d2 6 d3 2\n')
        p = s.recv(1024)
        if eval(p) != {'src':45, 'd1':2, 'd2':11, 'd3':2}:
            return 'wrong transaction return message '+repr(p)

def test_exit():
    assert not run_test_with_server(case_exit)

def case_exit(port):
    from socket import socket, AF_INET, SOCK_STREAM
    with socket(AF_INET, SOCK_STREAM) as s:
        s.connect(('localhost', port))
        s.send(b'EXIT\n')
        p = s.recv(1024)
        if p != b'': return 'EXIT should not send anything back'









def random_port():
    import socket
    with socket.socket() as s:
        s.bind(('', 0))
        return s.getsockname()[1]

def run_test_with_server(fn, should_block=False, timeout=5, port=None):
    from subprocess import Popen, DEVNULL
    import os
    import time
    import shutil
    try: from signal import CTRL_C_EVENT
    except: from signal import SIGINT as CTRL_C_EVENT
    
    # start server
    if not port: port = random_port()
    if os.name == 'nt':
        try: from subprocess import CREATE_NEW_PROCESS_GROUP
        except: CREATE_NEW_PROCESS_GROUP = 0
        if shutil.which('py'):
            python_exe = 'py'
        elif shutil.which('python'):
            python_exe = 'python'
        else:
            python_exe = 'python3'
        ws = Popen([python_exe, "wallet-server.py",'-p',str(port)], stderr=DEVNULL, stdout=DEVNULL, stdin=DEVNULL, creationflags=CREATE_NEW_PROCESS_GROUP)
    else:
        ws = Popen(["python3", "wallet-server.py",'-p',str(port)], stderr=DEVNULL, stdout=DEVNULL, stdin=DEVNULL)
    time.sleep(0.1)
    
    try: # test it
        return run_one_test(fn, should_block=should_block, args=(port,), timeout=timeout)
    finally: # close it when done
        ws.send_signal(CTRL_C_EVENT)
        
        try:
            ws.wait(timeout)
        except:
            ws.kill()
        
        
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

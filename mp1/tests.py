from subprocess import run
import re
from os.path import exists
from os import unlink, stat

usedfunc = re.compile(r"DeclRefExpr[^\n]*Function 0x[a-f0-9]+ '([^']*)'")

def check_for_tokens(filename, tokens, mode='allow'):
    """Use clang's AST to check what functions are being used
    mode='allow' gives a set of functions that may be used
    mode='ban' gives a set of functions that may not be used
    returns a list of violations (an empty list if there are none)"""
    res = run(['clang','-Xclang','-ast-dump',filename], capture_output=True)
    used = set(usedfunc.findall(res.stdout.decode('utf-8')))
    filters = [re.compile(_) for _ in tokens if re.search(r'[^*$\\\[\]()]', _)]
    tokens = [_ for _ in tokens if not re.search(r'[^*$\\]', _)]
    errors = []
    if mode == 'allow':
        for token in used:
            if token in tokens or any(f.search(token) for f in filters): continue
            errors.append(token)
    else:
        for token in used:
            if token in tokens or any(f.search(token) for f in filters): errors.append(token)
    return errors


def test(command, expected, points, msg, msg2=None, timeout=None):
    """Tests one command-line invocation"""
    res = run(command, shell=True, capture_output=True, timeout=timeout)
    if res.stdout.decode('utf-8').strip() == expected.strip():
        print('✅',msg)
        return points
    elif msg2:
        print('❌',msg2)
        return 0
    else:
        print('❌ wrong output for',msg)
        return 0
    
def valgrind(command, points, msg, msg2=None, timeout=None):
    """Uses valgrind on one command-line invocation"""
    if exists('.leaks'): unlink('.leaks')
    res = run(f'valgrind --trace-children=yes --leak-check=full -q --log-file=.leaks {command}', shell=True, capture_output=True, timeout=timeout)
    if exists('.leaks') and stat('.leaks').st_size > 2:
        print('❌ valgrind',msg2 if msg2 else msg)
        # print(open('.leaks').read())
        unlink('.leaks')
        return 0
    else:
        print('✅ valgrind',msg)
        return points
        


score = 0

try:

    banned = check_for_tokens('dll.c',['^dll', '^str', '^mem', 'malloc', 'calloc', 'realloc', 'free'])
    if banned:
        print('❌ library function limitations violated by', ', '.join(banned))
        quit()
    else:
        print('✅ library function limitations')
        score += 1


    run(['make','--silent','clean'])
    run(['make','--silent','dll_c'])

    if not exists('dll_c'):
        print('❌ failed to build with make')
        quit()
    else:
        print('✅ built without warnings or errors')
        score += 1

    # Main test cases are here:
    score += test(
        './dll_c e', # command we run
        'e', 1, # what we expect as the result, and how many points to award
        'single character argument') # message to show

    score += test(
        './dll_c e f', 
        'e f', 1, 
        'two single character arguments')

    score += test(
        './dll_c one', 
        'eno', 1, 
        'single word argument')

    score += test(
        './dll_c one two three four five six', 
        'eno owt eerht ruof evif xis', 1, 
        'several arguments')
    

    score += test(
        './dll_c --help', 
        'This program is a simple linked-list demo; run it with arguments.', 1, 
        'found "--help" when it is the only argument', 
        'did not find "--help" when it is the only argument')

    score += test('./dll_c one --help two', 
        'This program is a simple linked-list demo; run it with arguments.', 1, 
        'found "--help" when it is one of several arguments', 
        'did not find "--help" when it is one of several arguments')
    
    score += valgrind('./dll_c 3.14159', 1, 'one argument')
    score += valgrind('./dll_c Now I, even I, would celebrate', 1, 'six arguments')
    
finally:
    print(f"SCORE: {score} / 10")



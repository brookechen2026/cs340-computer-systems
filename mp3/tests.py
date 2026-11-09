from subprocess import run
import re
from os.path import exists
from os import unlink, stat

def test(command, expected, points, msg, msg2=None, timeout=None, retcode=False):
    """Tests one command-line invocation"""
    try:
        res = run(command, shell=True, capture_output=True, timeout=timeout)
    except BaseException as ex:
        print('❌',msg2,ex)
        return 0
    
    if retcode: out = res.returncode
    else: out = res.stdout.decode('utf-8').replace('\r\n','\n').replace('\r','\n')
    
    if isinstance(expected, str): good = expected.strip() == out.strip()
    elif isinstance(expected, int): good = expected == out
    else: good = expected(out)
    
    if good:
        print('✅',msg)
        return points
    else:
        print('❌',msg2 if msg2 else msg)
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
multiplier = [1, 0.9, 0.8, 0.75]

try:
    run(['make','--silent','clean'])
    run(['make','--silent'])

    if exists('pngchunklist'):
        score += 5; print('✅ pngchunklist compilation')

        score += test(
            './pngchunklist img/onered.png', # command to run
            lambda s: len(re.findall(r'^\S', s, re.M)) == 3, 5, # check and score
            'chunk count for minimal example', # message
            timeout=2) # maximum number of seconds to run this test

        score += test(
            './pngchunklist img/onered.png',
            lambda s: re.findall(r'^\S[^\n]*', s, re.M) == ['IHDR 13','IDAT 12','IEND 0'], 10,
            'chunk list for minimal example',
            timeout=2)

        score += test(
            './pngchunklist img/favicon.png',
            lambda s: len(re.findall(r'^\S', s, re.M)) == 9, 5,
            'chunk count for larger example',
            timeout=2)

        score += test(
            './pngchunklist img/favicon.png',
            lambda s: re.findall(r'^\S[^\n]*', s, re.M) == ['IHDR 13', 'gAMA 4', 'cHRM 32', 'bKGD 2', 'tIME 7', 'IDAT 6915', 'tEXt 37', 'tEXt 37', 'IEND 0'], 10,
            'chunk list for larger example',
            timeout=2)
        
        if not valgrind('./pngchunklist img/favicon.png', 1, 'pngchunklist', timeout=4):
            del multiplier[0]
    else:
        print("❌ pngchunklist compilation")
    
    
    if exists('extractuiucchunk'):
        score += 5; print('✅ extractuiucchunk compilation')
        
        if exists('tmp'): unlink('tmp')

        score += test(
            './extractuiucchunk img/onered.png tmp',
            lambda s: not exists('tmp'), 2,
            'extraction of file with no uiuc chunk',
            timeout=2)

        score += test(
            './extractuiucchunk img/hidden1.png tmp',
            lambda s: exists('tmp'), 3,
            'extraction of file with uiuc chunk',
            timeout=2)
        score += test(
            'cat tmp',
            'CS 340 logo\n', 5,
            'extract correct file contents',
            timeout=None)
        
        if exists('tmp'): unlink('tmp')
        
        run('./extractuiucchunk img/hidden2.png tmp', # run without checking anything
            shell=True, capture_output=True, timeout=2) 
        score += test(
            './pngchunklist tmp',
            lambda s: re.findall(r'^\S[^\n]*', s, re.M) == ['IHDR 13', 'pHYs 9', 'IDAT 5916', 'uiuc 633426', 'IEND 0'], 5,
            'chunk list for PNG extracted from another PNG',
            timeout=2)

        run('./extractuiucchunk tmp tmp2',
            shell=True, capture_output=True, timeout=2) 
        try: # checks the length and a few bytes of the extracted block
            clen = stat('tmp2').st_size
            with open('tmp2', 'rb') as f:
                f.seek(633248)
                sample = f.read(7)
            assert (clen, sample) == (633426, b'gaen[pf')
            score += 5; print('✅ correct nested hidden block')
        except:
            print('❌ incorrect nested hidden block')
        
        if exists('tmp'): unlink('tmp')
        if exists('tmp2'): unlink('tmp2')

        if not valgrind('./extractuiucchunk img/hidden2.png tmp', 1, 'extractuiucchunk', timeout=4):
            del multiplier[0]
        if exists('tmp'): unlink('tmp')

    else:
        print("❌ extractuiucchunk compilation")



    if exists('insertuiucchunk'):
        score += 0; print('✅ insertuiucchunk compilation')

        run('echo "this is a test" >> tmp', shell=True, capture_output=True)
        
        score += test(
            './insertuiucchunk img/onered.png tmp tmp2',
            lambda s: exists('tmp2'), 2,
            'insert text into PNG creates file',
            timeout=2)
        if exists('tmp2') and stat('tmp2').st_size == 96:
            score += 3; print('✅ created file size')
        else: print('❌ created file size')

        score += test(
            './pngchunklist tmp2',
            lambda s: 'uiuc 15' in s, 3,
            'has uiuc chunk',
            timeout=2)

        score += test(
            './pngchunklist tmp2',
            lambda s: re.search('IHDR.*uiuc.*IEND', s, re.S), 2,
            'uiuc chunk between two others',
            timeout=2)
        
        if exists('tmp2') and b'\x75\x69\x75\x63\x74\x68\x69\x73\x20\x69\x73\x20\x61\x20\x74\x65\x73\x74\x0a\xa6\xc1\x3e' in open('tmp2','rb').read():
            score += 5; print('✅ correct CRC checksum aded')
        else: print('❌ incorrect CRC checksum aded')


        score += test(
            './insertuiucchunk tmp2 img/onered.png tmp3',
            lambda s: exists('tmp3') and stat('tmp3').st_size == 150, 5,
            'replace uiuc chunk file size',
            timeout=2)

        score += test(
            './pngchunklist tmp3',
            lambda s: re.search('IHDR.*uiuc.*IEND', s, re.S), 5,
            'uiuc chunk between two others',
            timeout=2)
        
        if exists('tmp2') and b'\x49\x45\x4e\x44\xae\x42\x60\x82\xeb\xc9\xcb\x51' in open('tmp3','rb').read():
            score += 5; print('✅ correct CRC checksum aded')
        else: print('❌ incorrect CRC checksum aded')

        score += test(
            './extractuiucchunk tmp2 tmp4',
            lambda s: exists('tmp4') and open('tmp','rb').read() == open('tmp4','rb').read(), 5,
            'insert then extract text',
            timeout=2)

        score += test(
            './extractuiucchunk tmp3 tmp5',
            lambda s: exists('tmp5') and open('img/onered.png','rb').read() == open('tmp5','rb').read(), 5,
            'insert then extract image',
            timeout=2)


        if not valgrind('./insertuiucchunk tmp2 img/onered.png tmp6', 1, 'insertuiucchunk', timeout=4):
            del multiplier[0]
        unlink('tmp')
        for i in range(1,8):
            if exists(f'tmp{i}'): unlink(f'tmp{i}')

    else:
        print("❌ insertuiucchunk compilation")

    
    if score == 100 and multiplier[0] == 1:
        print("🙌 perfect score; checking for extra credit")
        
        extra = 0
        
        
        extra += test(
            './pngchunklist img/oneredd.png',
            2, 0.5,
            'non-existant file',
            retcode=True, timeout=2)
        
        extra += test(
            './pngchunklist Makefile',
            3, 0.5,
            'header error file',
            retcode=True, timeout=2)

        extra += test(
            './pngchunklist img/bad1.png',
            4, 0.5,
            'IHDR not first',
            retcode=True, timeout=2)

        extra += test(
            './pngchunklist img/bad2.png',
            4, 0.5,
            'no IHDR',
            retcode=True, timeout=2)

        extra += test(
            './pngchunklist img/bad3.png',
            4, 0.5,
            'something after IEND',
            retcode=True, timeout=2)

        extra += test(
            './pngchunklist img/bad4.png',
            4, 0.5,
            'no IEND',
            retcode=True, timeout=2)

        extra += test(
            './pngchunklist img/bad5.png',
            5, 0.5,
            'bad CRC',
            retcode=True, timeout=2)

        extra += test(
            './pngchunklist img/bad6.png',
            5, 0.5,
            'bad CRC',
            retcode=True, timeout=2)

        extra += test(
            './pngchunklist img/onered.png',
            lambda s: re.findall(r'^\t[^\n]*', s, re.M) == ['\twidth 1','\theight 1'], 0.5,
            'width and height equal',
            timeout=2)

        run('./extractuiucchunk img/hidden2.png tmp', shell=True, capture_output=True, timeout=3)
        extra += test(
            './pngchunklist tmp',
            lambda s: re.findall(r'^\t[^\n]*', s, re.M) == ['\twidth 262','\theight 146'], 0.5,
            'width and height not equal',
            timeout=2)
        
        if exists('tmp'): unlink('tmp')
        
        if extra == 5:
            score += 5
        else:
            print("❌ extra credit graded all-or-nothing")
            

except BaseException as ex:
    print('❌ tests.py crashed with error', repr(ex))
finally:
    print(f"SCORE: {score} / 100")
    print(f"SCORE MULTIPLIER: {multiplier[0]}")



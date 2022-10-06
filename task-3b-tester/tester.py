#!/usr/bin/env python3

import subprocess, os, atexit, requests, traceback, socket, http.client, contextlib, functools, time, stat
from unittest import skipIf
from sys import argv, exit, stdout as sys_stdout
from time import sleep
from signal import SIGINT
from hashlib import sha512
from contextlib import closing
from glob import glob
    
sout = None
serr = None
if len(argv) != 2:
    print('Usage: ./tester.py <path to task 3b base dir>')
    exit(1)


print('Waiting for g++ build (timeout 30s)...', end='')
try:
    cwd = os.getcwd()
    os.chdir(argv[1])
    gcc = subprocess.run(
        #g++ -std=c++17 -Wno-unused-variable -Wall -Werror -ggdb -O0 *.cpp -lavcodec -lavformat -lavutil -lm -o server 
        ['g++','-std=c++17','-ggdb','-Wno-unused-variable','-Wall','-Werror','-O0'] + glob('*.cpp') + ['-lavcodec','-lavformat','-lavutil','-lm','-o',os.path.join(cwd,'server')],
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
        timeout=30
    )
    if gcc.returncode == 0:
        print('OK.')
    else:
        print('failed.\n')
        print(gcc.stdout)
        print('\n== ABORT: gcc exited with nonzero code (%d) ==' % gcc.returncode)
        exit(1)
    os.chdir(cwd)
except subprocess.TimeoutExpired as e:
    print('timed out.\n')
    print(e.stdout)
    print('\n== ABORT: Timeout (30s) hit while waiting for g++ build ==')
    exit(1)


proc = subprocess.Popen(
    #valgrind --leak-check=full --suppressions=./suppression.supp ./server $(fname)
    ['valgrind','--leak-check=full','--suppressions=' + os.path.join(argv[1],'suppression.supp'),'./server'],
    stdout = subprocess.PIPE, stderr = subprocess.PIPE, text=True,
)
atexit.register(lambda p: p.kill(), proc)


print('Waiting for server to come up (5s)...')
sleep(5)
print('Checking if server survived startup... ', end='')
if proc.poll() is not None:
    print('it terminated.\n')
    
    try:
        (sout, serr) = proc.communicate(timeout=5)
        print(sout)
        print('')
        print(serr)
        print('\n== ABORT: Server process did not survive startup (exit code %d) ==' % proc.returncode)
    except subprocess.TimeoutExpired as e:
        print(e.stdout)
        print('')
        print(e.stderr)
        print('\n== ABORT: Server process did not survive startup ==')
    
    exit(1)

print('OK.')

print('Now starting test run.', end='\n\n')

# TEST FRAMEWORK STARTS HERE 

class _TestFailedException(Exception):
    def __init__(self, text):
        self.text = text
    def __str__(self):
        return self.text

class _TestSkippedException(Exception):
    pass
    

    
_Tests = []
_CurrentSection = None

def SECTION_START(name):
    global _CurrentSection
    _CurrentSection = []
    _Tests.append((name, _CurrentSection))

def Test(fn):
    _CurrentSection.append(fn)

def Fail(why):
    raise _TestFailedException(why)

def getHeaders(req):
    r = req.split("\r\n")[1:]
    res = dict()
    for line in r:
        if line == '':
            break
        k = line.split(": ")[0]
        res[k] = line.split(": ")[1]
    return res

def getBody(req):
    r = req.split("\r\n")[1:]
    body = ""
    found = False
    for line in r:
        if found:
            body += line + "\r\n"
        if line == '':
            found = True
    return body

def AssertBodyContains(req, value):
    try:
        b = getBody(req)
        if value not in b:
            raise _TestFailedException('Value (%s) was not present' % value)
    except:
        raise _TestFailedException('Could not get body or value (%s) was not present' % value)


def AssertEquals(desc, has, expect):
    if has != expect:
        raise _TestFailedException('Assertion of "%s" failed.\nObserved: %s\nExpected: %s' % (desc, repr(has), repr(expect)))

def AssertStatus(req, target):
    if req.split("\r\n")[0] != F"RTSP/1.0 {target}":
        raise _TestFailedException('Expected status (%s), but got (%s)' %
                                    (target, req))


def AssertHeaderExists(req, key):
    try:
        headers = getHeaders(req)
        present = headers[key]
        return present
    except KeyError:
        raise _TestFailedException('Expected RTSP "%s" header (with value "%s"). Header is not present.' % (key, value))

def AssertHeader(req, key, value):
    try:
        headers = getHeaders(req)
        present = headers[key]
        if present != str(value):
            raise _TestFailedException('RTSP header field "%s": expected "%s", got "%s".' % (key, value, present))
    except KeyError:
        raise _TestFailedException('Expected RTSP "%s" header (with value "%s"). Header is not present.' % (key, value))

def AssertHeaderContains(req, key, value):
    try:
        headers = getHeaders(req)
        present = headers[key]
        if not value in present:
            raise _TestFailedException('RTSP header field "%s" should contain "%s", got "%s".' % (key, value, present))
    except KeyError:
        raise _TestFailedException('Expected RTSP "%s" header (with value "%s"). Header is not present.' % (key, value))

def GetHeaderAssertExists(req, key):
    try:
        return getHeaders(req)[key]
    except KeyError:
        raise _TestFailedException('Expected RTSP "%s" header to exist. Header does not exist.' % key)

def Skip():
    raise _TestSkippedException

def SkipIf(c):
    if c:
        Skip()

class RawRequestResponse:
    def __init__(self, statusCode, statusText, headers, body):
        self.status_code = statusCode
        self.reason = statusText
        self.headers = headers
        self.content = body

def RawRequest(body, segmentInfo=None):
    def _printable_ascii(what, b):
        try:
            v = b.decode('ascii')
            if v.isprintable():
                return v
        except UnicodeDecodeError:
            pass
        Fail('Unexpected byte sequence in %s, not printable ASCII: %s' % (what, repr(b)))

    try:
        with closing(socket.create_connection(('127.0.0.1','8554'), 2)) as s:

            if segmentInfo is None:
                s.send(body.encode('ascii'))
            else:
                (segmentLength, segmentDelay) = segmentInfo
                first = False
                for segment in (body[i:i+segmentLength] for i in range(0, len(body), segmentLength)):
                    if not first:
                        first = True
                    else:
                        time.sleep(segmentDelay)
                    s.send(segment.encode('ascii'))

            content = b''
            while True:
                try:
                    c = s.recv(4096)
                except socket.timeout:
                    Fail('recv() timed out, maybe a socket was not closed?')
                except ConnectionResetError:
                    c = b''

                if c == b'':
                    break
                content += c
            
            if (len(content) < 14) or (content[8] != ord(' ')) or (content[12] != ord(' ')):
                Fail('Invalid RTSP response -- invalid status line:\n%s' % repr(content))
            
            AssertEquals('RTSP version', content[:8], b'RTSP/1.0')
            
            try:
                statusCode = int(content[9:12])
                if statusCode < 100 or statusCode > 999:
                    raise ValueError('Invalid RTSP Status %d' % statusCode)
            except ValueError:
                Fail('Invalid RTSP status "%s"' % repr(content[9:12]))


            idx = content.find(b'\r\n', 13)
            if idx < 0:
                Fail('Invalid RTSP response -- overlong status line:\n%s' % repr(content))
            
            statusText = _printable_ascii('RTSP status text', content[13:idx])
            
            headers = requests.structures.CaseInsensitiveDict()
            idx += 2
            while True:
                idxEnd = content.find(b'\r\n', idx)
                if idxEnd < 0:
                    Fail('Invalid RTSP response -- no termination of header section:\n%s' % repr(content))
                
                headerLine = _printable_ascii('RTSP header line', content[idx:idxEnd])
                
                idx = idxEnd+2
                if headerLine == '':
                    break

                colonIdx = headerLine.find(':')
                if colonIdx < 0:
                    Fail('Invalid RTSP header line:\n%s' % repr(headerLine))
                headers[headerLine[:colonIdx]] = headerLine[colonIdx+1:].lstrip()
            
            return RawRequestResponse(statusCode, statusText, headers, content[idx:])
        
    except _TestFailedException:
        raise
    except Exception:
        raise RuntimeError('Failed to make raw request')

# Perform an RTSP request. <verb> should be one of the RTSP request types. <path> holds the requested resource's name. 
# <CSeq> can be set arbitrarily and should be reflected by the server. <additional_headers> will hold all headers that
# exceed the CSeq header. <body> can be used to add a request body if needed.
# <close> and <sock> are used to dictate the behaviour of the underlying socket connection.
# if close is set to False, the socket will remain open and can be re-used in subsequent 
# calls to doRTSPRequest. The function will return the socket as a second return value if
# <close> is set to False.
# Setting <sock> to a socket will cause the function to re-use the specified socket connection. 
def doRTSPRequest(verb, path, CSeq, additional_headers=[], body=None, close=True, sock = None):
    s = None
    if sock:
        s = sock
    else:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect(("127.0.0.1", 8554))
    payload = F"{verb} {path} RTSP/1.0\r\nCSeq: {CSeq}\r\n"
    for h in additional_headers:
        payload += h+"\r\n"
    payload += "\r\n"
    if body != None:
        payload += body
        payload += "\r\n"
    s.settimeout(10)
    s.send(bytes(payload, "UTF-8"))
    res = s.recv(4096)
    res = res.decode("UTF-8")
    if close:
        s.close()
        return res
    else:
        return res, s

# Receive RTSP packets from a specified port.
# Returns the duration of the stream
# You can use the returned value to determine if your rate limiting works
def recvRTPPackets(client_port):
    s = None
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind(("127.0.0.1", client_port))
    s.settimeout(1.0)
    print(F"got connection on {client_port}")
    now = time.time()
    while True:
        r = ''
        try:
            r = s.recv(1024)
        except:
            print("Reached end of UDP stream")
            r = ''
        if r == '':
            break
    duration = time.time()-now
    print(duration)
    return duration

# END OF TEST FRAMEWORK, ACTUAL TESTS BELOW


SECTION_START('Basic functionality')
# Will be set to true if describe works.
hasDescribe = False
# Will be set to true if the basic setup test succeeds.
hasSetup = False
# Will hold the session ID after the basic setup test
sessionID = -1

@Test
def BasicOPTIONS():
    resp = doRTSPRequest("OPTIONS", "*", 1)
    print(resp)
    # Check that the status code is correct
    AssertStatus(resp, "200 OK")
    # CSeq should always be the value that was specified in the request
    AssertHeader(resp, "CSeq", 1)
    # The response must contain all implemented RTSP methods.
    # The order in which the methods are listed is arbitrary.
    AssertHeaderContains(resp, "Public", "OPTIONS")
    AssertHeaderContains(resp, "Public", "DESCRIBE")
    AssertHeaderContains(resp, "Public", "SETUP")
    AssertHeaderContains(resp, "Public", "PLAY")
    AssertHeaderContains(resp, "Public", "TEARDOWN")

@Test
def BasicDESCRIBE():
    resp = doRTSPRequest("DESCRIBE", "rtsp://127.0.0.1:8554/sample.mp3", 1)
    print(resp)
    AssertStatus(resp, "200 OK")
    AssertHeader(resp, "CSeq", 1)
    AssertHeader(resp, "Content-Type", "application/sdp")
    AssertHeaderExists(resp, "Content-Length")
    AssertBodyContains(resp, "m=audio 0 RTP/AVP 14")
    AssertBodyContains(resp, "o=- 0 0 IN IP4 127.0.0.1")
    global hasDescribe
    hasDescribe = True


@Test
def BasicSETUP():
    skipIf(hasDescribe == False, "Describe not implemented or not working")
    resp = doRTSPRequest("SETUP", "rtsp://127.0.0.1:8554/sample.mp3", 1, ["Transport: RTP/AVP/UDP;unicast;client_port=4588-4589"])
    print(resp)
    AssertStatus(resp, "200 OK")
    global sessionID
    sessionID = AssertHeaderExists(resp, "Session")
    AssertHeaderContains(resp, "Transport", "RTP/AVP/UDP;unicast;client_port=4588-4589")
    global hasSetup
    hasSetup = True

@Test
def BasicPLAY():
    print("----------")
    skipIf(hasSetup == False, "Setup not working")
    resp,sock = doRTSPRequest("SETUP", "rtsp://127.0.0.1:8554/sample2.mp3", 1, ["Transport: RTP/AVP/UDP;unicast;client_port=4588-4589"], close=False)
    print(resp)
    sess = AssertHeaderExists(resp, "Session")
    resp = doRTSPRequest("PLAY", "rtsp://127.0.0.1:8554/sample2.mp3", 1, [F"Session: {sess}"],sock=sock)
    print(resp)
    AssertStatus(resp, "200 OK")
    duration = recvRTPPackets(4588)
    if duration > 5 or duration < 3:
        raise _TestFailedException(F'Duration {duration} does not fit to the actual media duration!')

# Feel free to add testcases here!
# We provide you only basic tests, make sure that you test 
# all the required functionality 
# END OF TEST DEFINITIONS

_TestResults = []

for (sectionName, testData) in _Tests:
    print('== %s ==' % sectionName.strip())
    numStr = str(len(testData))
    fmtStr = ('[%%%dd/%s] "%%s"... ' % (len(numStr), numStr))
    
    nPass = 0
    nFail = 0
    nSkip = 0
    
    for (idx, testFn) in enumerate(testData, 1):
        testName = testFn.__name__
        print(fmtStr % (idx, testName), end='')
        sys_stdout.flush()
        try:
            testFn()
            print('OK.')
            nPass += 1
        except _TestFailedException as e:
            print('\x1b[2K\r\n(!!) ', end='')
            print(fmtStr % (idx, testName), end='FAIL.\n')
            print(str(e), end='\n\n')
            nFail += 1
        except _TestSkippedException:
            print('SKIP.')
            nSkip += 1
        except Exception:
            print('\x1b[2K\r\n(!!) ', end='')
            print(fmtStr % (idx, testName), end='FAIL.\n')
            traceback.print_exc()
            print('')
            nFail += 1
    
    _TestResults.append((sectionName, (nPass, nFail, nSkip)))
    print('')

print('Done running tests, terminating server... ', end='')
sleep(1)
proc.send_signal(SIGINT)
try:
    (stdout, stderr) = proc.communicate(timeout=30)
    print('OK.\n')
    print('Valgrind output below:')
    print(stderr)
    print('\n== Test summary: ==')
    print(stdout)
    for (category, (nPass, nFail, nSkip)) in _TestResults:
        print('%s: %2d passed, %2d failed, %2d skipped' % (category, nPass, nFail, nSkip))
except subprocess.TimeoutExpired as e:
    print('timed out?\n')
    print(e.stdout)
    print('')
    print(e.stderr)
    print('\n== ABORT: Server process failed to terminate on SIGINT ==')
    exit(1)

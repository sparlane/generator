char = newSystemType('char')
cchar = newSystemType('const char')
uchar = newSystemType('unsigned char')
int = newSystemType('int')
size = newSystemType('size_t')
uint8 = newSystemType('uint8_t')
uint16 = newSystemType('uint16_t')
uint32 = newSystemType('uint32_t')
uint64 = newSystemType('uint64_t')
bool = newSystemType('bool')

pchar = newPointer(char, 'free')
pcchar = newPointer(cchar, 'free')
puchar = newPointer(uchar, 'free')
pvoid = newPointer(newSystemType('void'),'free')
sockaddr = newPointer(newSystemType('struct sockaddr'),'free')
time = newSystemType('time_t')

require 'us/common'
require 'us/thread'
require 'us/jq'
require 'us/lua'
require 'us/datacoding'
require 'us/event'
require 'us/encryption'
require 'us/delay'
require 'us/network'
require 'us/base/_core'

m = moduleCreate('scott', 'lib', 'us_', 'us_')
t = m:newType('test')

t:memberAdd(newSystemType('int'), 'test')
t:memberAdd(newPointer(newSystemType('int'), 'free'), 'scott')

t2 = m:newType('test2')

t2:memberAdd(t, 't')

t3 = m:newType('arrayTest')
t3:memberAdd(newArray(newSystemType('int')), 'a')

f = t3:functionCreate('scott',newSystemType('bool'))
f:paramAdd(t,'t')

-- m:addInclude('tester.h')

t4 = m:newType('arrayTest2')
t4:memberAdd(newArray(t3), 'testA')

-- try a queue
q1 = m:newQueue(newSystemType('int'), 'testque')

-- try a bst
b1 = m:newBST(q1, 'testbst', false)
b2 = m:newBST(t, 'testbst2', true)

fp = m:newFunctionPointer('test_int_giving',newSystemType('int'))
fp:paramAdd(t,'test')

ts = m:newType('finalTest')
ts:memberAdd(fp,'func')

ts:memberAdd(t4, 'testB')

m = moduleCreate('example', 'lib', 'clang_', 'clang_')

t = m:newType('test')

t:memberAdd(newSystemType('int'), 'test')

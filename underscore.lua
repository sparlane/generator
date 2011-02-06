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

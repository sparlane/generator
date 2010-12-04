m = moduleCreate('scott', 'lib', 'us_', 'us_')
t = typeCreate(m, 'test')

t:memberAdd(externTypeCreate('int', 'test'))
t:memberAdd(pointerCreate(externTypeCreate('int', ''), 'scott', 'free'))

t2 = typeCreate(m, 'test2')

t2:memberAdd(objectCreate(t, 't'))

t3 = typeCreate(m, 'arrayTest')
t3:memberAdd(arrayCreate(externTypeCreate('int',''), 'a'))

f = t3:functionCreate('scott',externTypeCreate('bool',''))
f:paramAdd(objectCreate(t,'t'))

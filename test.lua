int = newSystemType('int')
pchar = newPointer(newSystemType('char'),'free')

mod = moduleCreate('test','','t_','t_')

type1 = mod:newType('a')
type1:memberAdd(pchar, 't')
type1:memberAdd(int,'l')

type2 = mod:newType('b')
type2:memberAdd(type1, 'test')

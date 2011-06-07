m = moduleCreate('common', 'lib', 'us_', 'us_')

m:addInclude('stdint.h')

sl = m:newType('string_length',true,true)

sl:memberAdd(pchar,'string',true,true)
sl:memberAdd(uint32,'length',true,true)

sp = m:newType('string_pair',true,true)

sp:memberAdd(pchar,'first',true,true)
sp:memberAdd(pchar,'second',true,true)



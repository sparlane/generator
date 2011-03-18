m = moduleCreate('common', 'lib', 'us_', 'us_')

sl = m:newType('string_length',true,true)

sl:memberAdd(pchar,'string',true)
sl:memberAdd(uint32,'length',true)



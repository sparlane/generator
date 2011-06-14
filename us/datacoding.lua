mdc = moduleCreate('datacoding', 'lib', 'us_', 'us_')

mdc:addInclude('stdint.h')

dc_data = mdc:newType('datacoding_data',true,true)
dc_data:memberAdd(uint8,'ident',true,true)
dc_data:memberAdd(uint32,'length',true,true)
dc_data:memberAdd(pvoid,'data',true,true)

dc = mdc:newType('datacoding')
dc:memberAdd(newArray(dc_data),'packets',true,false)
dc:memberAdd(uint32,'next',true,false)
dc:memberAdd(uint32,'length',true,false)
dc:memberAdd(pchar,'string',true,false)

dc_import_string = dc:functionCreate('import_string',bool)
dc_import_string:paramAdd(sl,'string')

dc_add = dc:functionCreate('add',bool)
dc_add:paramAdd(dc_data,'data')

dc_add_sl = dc:functionCreate('add_sl',bool)
dc_add_sl:paramAdd(uint8,'ident')
dc_add_sl:paramAdd(sl,'string')

dc_add_sint = dc:functionCreate('add_sint',bool)
dc_add_sint:paramAdd(uint8,'ident')
dc_add_sint:paramAdd(newSystemType('signed long long'),'sint')

dc_add_uint = dc:functionCreate('add_uint',bool)
dc_add_uint:paramAdd(uint8,'ident')
dc_add_uint:paramAdd(newSystemType('unsigned long long'),'uint')

dc_add_blank = dc:functionCreate('add_blank',bool)
dc_add_blank:paramAdd(uint8,'ident')

dc_add_string = dc:functionCreate('add_string',bool)
dc_add_string:paramAdd(uint8, 'ident')
dc_add_string:paramAdd(uint32, 'length')
dc_add_string:paramAdd(pchar, 's')

dc_get = dc:functionCreate('get',dc_data)

dc_reset = dc:functionCreate('reset_get',bool)

dc_string = dc:functionCreate('string',sl)


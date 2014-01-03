mcore = moduleCreate('core', 'lib', 'us_', 'us_')

mcore:addInclude('string.h')

core_enum_mesg_ids = mcore:newEnum('US_CORE_MESG_ID')
core_enum_mesg_ids:valueAdd('IDENT')
core_enum_mesg_ids:valueAdd('CONFIG')
core_enum_mesg_ids:valueAdd('NEXTID')
core_enum_mesg_ids:valueAdd('ASSIGN_ROLE')


core_conn_type = mcore:newType('core_conn',true,true)

core_callback_lost = mcore:newFunctionPointer('callback_lost',bool)
core_callback_change = mcore:newFunctionPointer('callback_change',bool)
core_callback_change:paramAdd(int, 'role')
core_callback_change:paramAdd(pchar, 'addr')
core_callback_other = mcore:newFunctionPointer('callback_other',bool)
core_callback_other:paramAdd(int, 'role')
core_callback_other:paramAdd(net_message, 'mesg')


core_conn_type:memberAdd(pchar, 'addr', true, true)
core_conn_type:memberAdd(net_conn_type, 'conn_type', true, true)
core_conn_type:memberAdd(char, 'type', true, true)
core_conn_type:memberAdd(core_callback_lost, 'lost_cb', true, true)
core_conn_type:memberAdd(core_callback_change, 'change_cb', true, true)
core_conn_type:memberAdd(core_callback_other, 'other_cb', true, true)
core_conn_type:memberAdd(bool, 'shutdown', true)
core_conn_type:memberAdd(net_connection, 'primary', true)
core_conn_type:memberAdd(net_connection, 'secondary', true)
core_conn_type:memberAdd(dc, 'ident', true)

core_send = core_conn_type:functionCreate('send', bool)
core_send:paramAdd(sl, 'data')
core_send:paramAdd(uint32, 'ID')

core_suspend = core_conn_type:functionCreate('suspend', bool)
core_resume = core_conn_type:functionCreate('resume', bool)

core_update_info = core_conn_type:functionCreate('update_info', bool)
core_update_info:paramAdd(uchar, 'type')
core_update_info:paramAdd(pchar, 'ident')
core_update_info:paramAdd(int, 'port')
core_update_info:paramAdd(uint32, 'ID')


mev = moduleCreate('event', 'lib', 'us_', 'us_')

event_occured = mev:newFunctionPointer('occured',bool)
event_invalid = mev:newFunctionPointer('invalid',bool)

event = mev:newType('event',false,false)
event_set = mev:newType('event_set',false,false)

event:memberAdd(int,'fd',true,true)
event:memberAdd(event_occured,'occured',true,true)
event:memberAdd(event_invalid,'invalid',true,true)
event:memberAdd(pvoid,'priv')
event:memberAdd(int,'event')
event:memberAdd(event_set,'es')

es_bst = mev:newBST(event,'event_set_bst',true)

event_set:memberAdd(es_bst,'all',true,false, 'us_event_set_bst_create()')
event_set:memberAdd(bool,'updated',false)
event_set:memberAdd(thread,'ewatcher',true,false)
event_set:memberAdd(jq,'jq',true,true)

event_occured:paramAdd(event, 'e')
event_invalid:paramAdd(event, 'e')

es_start = event_set:functionCreate('start',bool)

es_add = event_set:functionCreate('add',bool)
es_add:paramAdd(event,'e')

es_bst_fe_cb = mev:newFunctionPointer('bst_foreach_cb',bool)
es_bst_fe_cb:paramAdd(event, 'e')
es_bst_fe_cb:paramAdd(pvoid, 'priv')

es_bst_foreach = es_bst:functionCreate('foreach', bool)
es_bst_foreach:paramAdd(es_bst_fe_cb,'cb')
es_bst_foreach:paramAdd(pvoid,'priv')

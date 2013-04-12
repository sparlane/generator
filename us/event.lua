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

event_set:memberAdd(mev:newBST(event,'event_set_bst',true),'all',true,false)
event_set:memberAdd(bool,'updated',true,false)
event_set:memberAdd(thread,'ewatcher',true,false)
event_set:memberAdd(jq,'jq',true,true)

es_start = event_set:functionCreate('start',bool)

es_add = event_set:functionCreate('add',bool)
es_add:paramAdd(event,'e')

mdelay = moduleCreate('delay', 'lib', 'us_', 'us_')

delay_callback = mev:newFunctionPointer('callback',bool)

delay = mdelay:newType('delay',false,false)
delay_set = mdelay:newType('delay_set',false,false)

delay:memberAdd(int,'fd',true,true)
delay:memberAdd(delay_callback,'handler',true,true)
delay:memberAdd(pvoid,'priv')
delay:memberAdd(uint64,'delay')
delay:memberAdd(delay,'next',true,false)

delay_set:memberAdd(delay,'first',true,false)
delay_set:memberAdd(jq,'jq',true,true)
delay_set:memberAdd(mdelay:newConditional("my_cond"),'cond',true,false)
delay_set:memberAdd(thread,'edelay',true,false)

es_start = delay_set:functionCreate('start',bool)

es_add = delay_set:functionCreate('add',bool)
es_add:paramAdd(delay,'d')

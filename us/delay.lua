mdelay = moduleCreate('delay', 'lib', 'us_', 'us_')

delay = mdelay:newType('delay',false,false)
delay_set = mdelay:newType('delay_set',false,false)

delay:memberAdd(int,'fd',true,true)
delay:memberAdd(jq_cb_fp,'handler',true,true)
delay:memberAdd(pvoid,'priv',true, true)
delay:memberAdd(uint64,'delay', true, true)
delay:memberAdd(delay,'next',true,false)

delay_set:memberAdd(delay,'first',true,false)
delay_set:memberAdd(jq,'jq',true,true)
delay_set:memberAdd(mdelay:newConditional("cond_delay"),'cond',true,false)
delay_set:memberAdd(thread,'edelay',true,false)

ds_start = delay_set:functionCreate('start',bool)
ds_start:paramAdd(jq, 'jq')

ds_add = delay_set:functionCreate('add',bool)
ds_add:paramAdd(delay,'d')

m = moduleCreate('job_queue', 'lib', 'us_', 'us_')

job = m:newType('job',true)
jq = m:newType('job_queue')

jq_cb_fp = m:newFunctionPointer('callback',bool)
jq_cb_fp:paramAdd(pvoid,'data')

job:memberAdd(jq_cb_fp,'cb',true,true)
job:memberAdd(pvoid,'data',true,true)
job:memberAdd(time,'t',true,true)

jq:memberAdd(m:newHeap(job,'job_heap',false, false),'pending', true, false, 'us_job_heap_create()')
jq:memberAdd(newArray(thread),'handlers', true)
jq:memberAdd(m:newConditional("cond_jq"),'jobs_cond', true, false, 'us_cond_jq_create()')

jq_add_workers = jq:functionCreate('add_workers',bool)
jq_add_workers:paramAdd(int,'w')

jq_enqueue = jq:functionCreate('enqueue', bool)
jq_enqueue:paramAdd(jq_cb_fp, 'handler')
jq_enqueue:paramAdd(pvoid,'data')

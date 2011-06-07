m = moduleCreate('job_queue', 'lib', 'us_', 'us_')

job = m:newType('job',true)
jq = m:newType('job_queue')

jq_cb_fp = m:newFunctionPointer('callback',bool)
jq_cb_fp:paramAdd(pvoid,'data')

job:memberAdd(jq_cb_fp,'cb')
job:memberAdd(pvoid,'data')

jq:memberAdd(m:newBST(job,'job_bst',true),'pending')
jq:memberAdd(newArray(thread),'handlers')
jq:memberAdd(m:newConditional("my_cond"),'jobs_cond')

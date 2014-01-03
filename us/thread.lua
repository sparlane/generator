tm = moduleCreate('thread', 'lib', 'us_', 'us_')

tm:addInclude('stdint.h')

thread = tm:newType('thread')

thread:memberAdd(newPointer(newSystemType('pthread_t'),'free'),'t',true,false,'malloc(sizeof(pthread_t))')
thread:memberAdd(uint32,'info',true,true)
thread:memberAdd(newSystemType('void *'),'priv',true,true)
thread:memberAdd(bool,'started',true,false,'false')

thread_start = thread:functionCreate('start',bool)

thread_fp = tm:newFunctionPointer('thread_func',bool)
thread_fp:paramAdd(thread,'t')

thread:memberAdd(thread_fp, 'func',true,true)

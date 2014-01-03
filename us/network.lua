mnet = moduleCreate('network', 'lib', 'us_', 'us_')

net_conn_type = mnet:newType('connection_type',true,true)
net_conn_type:memberAdd(bool,'IPv4',true,true)
net_conn_type:memberAdd(bool,'IPv6',true,true)

net_callback_dropped = mnet:newFunctionPointer('callback_dropped',bool)
net_callback_new_conn = mnet:newFunctionPointer('new_connection',bool)
net_callback_message = mnet:newFunctionPointer('message',bool)
net_callback_udp_message = mnet:newFunctionPointer('udp_message',bool)

net_connection = mnet:newType('connection',false,false)
net_connection:memberAdd(int,'fd', true)
net_connection:memberAdd(int,'port', true)
net_connection:memberAdd(net_callback_dropped,'dropped', false)
net_connection:memberAdd(net_callback_message,'message', false)
net_connection:memberAdd(bool,'us_connection', true)
net_connection:memberAdd(char,'breakchar', false)
net_connection:memberAdd(bool,'sending', true)
net_connection:memberAdd(bool,'receiving', true)
net_connection:memberAdd(bool,'encrypted', false)
net_connection:memberAdd(bool,'compressed', false)
net_connection:memberAdd(rc4,'send_key', false)
net_connection:memberAdd(rc4,'recv_key', false)
net_connection:memberAdd(pvoid,'priv', false)
net_connection:memberAdd(net_conn_type,'conn_type', true)
net_connection:memberAdd(sockaddr, 'sa_local')
net_connection:memberAdd(sockaddr, 'sock_addr')

net_connection_set = mnet:newType('connection_set',false,false)
net_connection_set:memberAdd(mnet:newBST(net_connection,'connection_set_bst',true),'receive',true,false)
-- net_connection_set:memberAdd(mnet:newHEAP(net_connection,'connection_set_heap'),false),'recvfrom',true,false)
net_connection_set:memberAdd(mnet:newConditional('my_cond'),'jobs_cond',true,false)
net_connection_set:memberAdd(thread,'receiver',true,false)
net_connection_set:memberAdd(newArray(thread),'receiving',true,false)
net_connection_set:memberAdd(bool,'updated')

net_listen = mnet:newType('listen',false,false)
net_listen:memberAdd(int,'fd',true)
net_listen:memberAdd(int,'port',true)
net_listen:memberAdd(net_callback_new_conn,'newconn',true)
net_listen:memberAdd(pvoid,'priv',true)
net_listen:memberAdd(bool,'us_connection', true)
net_listen:memberAdd(bool,'breakchar',false)
net_listen:memberAdd(net_conn_type,'conn_type',true)
net_listen:memberAdd(event_set,'es',true)
net_listen:memberAdd(net_callback_dropped,'dropped',false)
net_listen:memberAdd(net_callback_message,'message',false)

net_udp = mnet:newType('udp',false,false)
net_udp:memberAdd(int,'fd', true)
net_udp:memberAdd(int,'port', true)
net_udp:memberAdd(net_callback_udp_message,'message', true)
net_udp:memberAdd(pvoid,'priv', true)
net_udp:memberAdd(net_conn_type,'conn_type', true)
net_udp:memberAdd(event_set,'es', true)

net_message = mnet:newType('message',true,true)
net_message:memberAdd(net_connection,'connection', true)
net_message:memberAdd(bool,'compressed', true)
net_message:memberAdd(bool,'encrypted', true)
net_message:memberAdd(uint32,'ID', true)
net_message:memberAdd(sl,'data', true)

net_udp_message = mnet:newType('udp_message',true,true)
net_udp_message:memberAdd(net_udp, 'connection', true, true)
net_udp_message:memberAdd(sockaddr, 'sock_addr', true, true)
net_udp_message:memberAdd(sl, 'data', true, true)

-- Add the correct parameters to the callbacks
net_callback_dropped:paramAdd(net_connection, 'c')

net_callback_new_conn:paramAdd(net_listen, 'l')
net_callback_new_conn:paramAdd(net_connection, 'c')

net_callback_message:paramAdd(net_message, 'm')

net_callback_udp_message:paramAdd(net_udp_message, 'm')

-- function definitions
-- connection
net_connection_drop = net_connection:functionCreate('drop', bool)

net_connection_send = net_connection:functionCreate('send', bool)
net_connection_send:paramAdd(sl, 'sl')
net_connection_send:paramAdd(uint32, 'type')

net_connection_accept = net_connection:functionCreate('accept', bool)
net_connection_accept:paramAdd(int, 'fd')
net_connection_accept:paramAdd(event_set, 'es')
net_connection_accept:paramAdd(net_conn_type, 'conn_type')
net_connection_accept:paramAdd(net_callback_dropped, 'dropped')
net_connection_accept:paramAdd(net_callback_message, 'message')

net_connection_port_ip = net_connection:functionCreate('port_ip', bool)
net_connection_port_ip:paramAdd(pcchar, 'addr')
net_connection_port_ip:paramAdd(uint16, 'port')
net_connection_port_ip:paramAdd(net_conn_type, 'conn_type')
net_connection_port_ip:paramAdd(net_callback_dropped, 'dropped')
net_connection_port_ip:paramAdd(net_callback_message, 'message')
net_connection_port_ip:paramAdd(bool, 'us_proto')
net_connection_port_ip:paramAdd(event_set, 'es')

net_connection_port_ip = net_connection:functionCreate('port_addr', bool)
net_connection_port_ip:paramAdd(pcchar, 'addr')
net_connection_port_ip:paramAdd(uint16, 'port')
net_connection_port_ip:paramAdd(net_conn_type, 'conn_type')
net_connection_port_ip:paramAdd(net_callback_dropped, 'dropped')
net_connection_port_ip:paramAdd(net_callback_message, 'message')
net_connection_port_ip:paramAdd(bool, 'us_proto')
net_connection_port_ip:paramAdd(event_set, 'es')

net_connection_recv = net_connection:functionCreate('recv', net_message)

-- udp
net_udp_broadcast = net_udp:functionCreate('broadcast',bool)
net_udp_broadcast:paramAdd(sl, 'sl')
net_udp_broadcast:paramAdd(int, 'port')

net_udp_sendto = net_udp:functionCreate('sendto',bool)
net_udp_sendto:paramAdd(sl, 'sl')
net_udp_sendto:paramAdd(pchar, 'addr')
net_udp_sendto:paramAdd(int, 'port')

net_udp_sendto_sa = net_udp:functionCreate('sendto_sa', bool)
net_udp_sendto_sa:paramAdd(sl, 'sl')
net_udp_sendto_sa:paramAdd(sockaddr, 'sa')
net_udp_sendto_sa:paramAdd(size, 'sa_len')

net_udp_recvfrom = net_udp:functionCreate('recvfrom', net_udp_message)

net_udp_begin = net_udp:functionCreate('begin', bool)
net_udp_begin:paramAdd(uint16, 'port')
net_udp_begin:paramAdd(net_conn_type, 'conn_type')
net_udp_begin:paramAdd(net_callback_udp_message, 'message')
net_udp_begin:paramAdd(event_set, 'es')

net_listen_begin = net_listen:functionCreate('open_port',bool)
net_listen_begin:paramAdd(int, 'port')
net_listen_begin:paramAdd(pchar, 'addr')
net_listen_begin:paramAdd(net_callback_new_conn, 'conn')
net_listen_begin:paramAdd(net_conn_type, 'conn_type')
net_listen_begin:paramAdd(bool, 'us_proto')
net_listen_begin:paramAdd(event_set, 'es')
net_listen_begin:paramAdd(net_callback_dropped, 'dropped')
net_listen_begin:paramAdd(net_callback_message, 'message')

net_listen_occurred = net_listen:functionCreate('event_occurred', bool)

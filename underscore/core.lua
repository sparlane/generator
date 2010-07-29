moduleCreate('core', 'base')

module_connection_o = objectCreate('core', 'module_connected', true, false)
server_connection_o = objectCreate('core', 'server_connected', true, false)
module_component_o = objectCreate('core', 'module_component', true, false)
module_type_o = objectCreate('core', 'module_type', true, false)
server_type_o = objectCreate('core', 'server_type', true, false)
debug_client_o = objectCreate('core', 'debug_client', true, false)
state_o = objectCreate('core', 'state', true, false)

-- state_o, jq
-- state_o, es
-- state_o, ds
objectAddType(state_o, 'unsigned long', 'nextID')
objectAddArrayObject(state_o, module_type_o, 'module_types')
objectAddArrayObject(state_o, server_type_o, 'server_types')
-- state_o, udp
objectAddType(state_o, 'bool', 'primary')
-- state_o, otherConn
objectAddPointer(state_o, 'char', 'otherCore', 'free')
objectAddType(state_o, 'unsigned int', 'heartbeat')
objectAddPointer(state_o, 'char', 'core_policy', 'free')
objectAddPointer(state_o, 'char', 'server_policy', 'free')
objectAddPointer(state_o, 'char', 'module_policy', 'free')
objectAddType(state_o, 'size_t', 'core_policy_length')
objectAddType(state_o, 'size_t', 'server_policy_length')
objectAddType(state_o, 'size_t', 'module_policy_length')
objectAddArrayObject(state_o, debug_client_o, 'debuggers')



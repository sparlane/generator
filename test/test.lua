test1_o = objectCreate('scott','s1', true, true)
test2_o = objectCreate('scott','s2', true, false)
test3_o = objectCreate('scott','s3', false, false)

beta = objectCreate('scott','beta', true, true)

objectAddObject(test1_o, test2_o, 'a', false, false, {})
objectAddObject(test1_o, test2_o, 'b', true, false, {})
objectAddObject(test2_o, test3_o, 'a', true, true, {})

moduleAddInclude('scott', 'string.h') 
moduleAddDep('scott', 'err')

objectAddArrayObject(beta, test1_o, 'arraya')
objectAddArrayObject(beta, test2_o, 'arrayb')
objectAddArrayPointer(beta, 'char', 'arrayc', 'free')
objectAddPointer(beta, 'char', 'str1', 'free')
objectAddPointer(beta, 'char', 'str2', 'free', true, false, 'strdup("testing")')
objectAddType(beta, 'bool', 'hello1')
objectAddType(beta, 'bool', 'hello2', true, false, 'true')
objectAddType(beta, 'bool', 'hello3', true, true)

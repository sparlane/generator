men = moduleCreate('encryption', 'lib', 'us_', 'us_')

rc4 = men:newType('rc4')
rc4:memberAdd(uchar,'idx1',true,false)
rc4:memberAdd(uchar,'idx2',true,false)
rc4:memberAdd(newArray(uchar),'S',true,false)

rc4_setup = rc4:functionCreate('setup',bool)
rc4_setup:paramAdd(puchar,'key')
rc4_setup:paramAdd(size,'length')

rc4_crypt = rc4:functionCreate('crypt',uchar)
rc4_crypt:paramAdd(uchar,'c')

rc4_crypt_string = rc4:functionCreate('crypt_string',puchar)
rc4_crypt_string:paramAdd(puchar,'s')
rc4_crypt_string:paramAdd(size,'length')


import sys

import verthash

teststart = '000000203a297b4b7685170d7644b43e5a6056234cc2414edde454a87580e1967d14c1078c13ea916117b0608732f3f65c2e03b81322efc0a62bcee77d8a9371261970a58a5a715da80e031b02560ad8'

with open('verthash.dat', 'rb') as f:
    datfile = f.read()

if sys.version_info[0] < 3:
    testbin = teststart.decode('hex')
else:
    testbin = bytes.fromhex(teststart)

hash_bin = verthash.getPoWHash(testbin, datfile)

if sys.version_info[0] < 3:
    print(hash_bin.encode('hex'))
else:
    print(hash_bin.hex())
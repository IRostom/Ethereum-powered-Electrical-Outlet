Import('env')
from base64 import b64decode

#
# Dump build environment (for debug)
# print env.Dump()
#

env.Replace(UPLOADOTACMD='node .\uploaderjs\index.js --board '+"$BOARD_MCU"+' --binary '+"$SOURCE "+ (('--partitions '+env['FLASH_EXTRA_IMAGES'][1][1]) if (env["BOARD_MCU"]=='esp32') else "") +' --host ' + "$UPLOAD_PORT")

# uncomment line below to see environment variables
# print ARGUMENTS

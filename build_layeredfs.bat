set PATH=%PATH%;C:\devkitPro\devkitARM\bin
del plugin\source\autogen.h
ctrtool --decompresscode -t exefs --exefsdir=workdir\exefs workdir\exefs.bin
copy workdir\exefs\code.bin workdir\dump.bin
locate.py workdir\dump.bin
cd plugin
build.py
cd ..
copy plugin\payload.bin workdir\layeredfs.plg

pause
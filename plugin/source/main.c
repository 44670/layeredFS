
#include "global.h"
#include "autogen.h"

#define O_RDONLY 1
#define O_WRONLY 2
#define O_RDWR   3
#define O_CREAT  4
#define REG(x)   (*(volatile u32*)(x))
#define REG8(x)  (*(volatile  u8*)(x))
#define REG16(x) (*(volatile u16*)(x))
#define SW(addr, data)  *(u32*)(addr) = data

Handle fsUserHandle;
FS_archive sdmcArchive = {0x9, (FS_path){PATH_EMPTY, 1, (u8*)""}};

typedef u32 (*fsRegArchiveTypeDef)(u8*, u32, u32) ;
typedef u32 (*userFsTryOpenFileTypeDef)(u32, u16*, u32);
typedef u32 (*fsMountArchiveTypeDef)(u32*, u32);


RT_HOOK	regArchiveHook;
RT_HOOK userFsTryOpenFileHook;

u32 fsRegArchiveCallback(u8* path, u32 ptr, u32 zero) {
	u32 ret, ret2;
	static u32 isFisrt = 1;
	u32 sdmcArchive = 0;

	ret = ((fsRegArchiveTypeDef)regArchiveHook.callCode)(path, ptr, zero);
	if (isFisrt) {
		isFisrt = 0;
		((fsMountArchiveTypeDef)fsMountArchive)(&sdmcArchive, 9);
		nsDbgPrint("sdmcArchive: %08x\n", sdmcArchive);
		if (sdmcArchive) {
			ret2 = ((fsRegArchiveTypeDef)regArchiveHook.callCode)("ram:", sdmcArchive, 0);
			nsDbgPrint("regArchive ret: %08x\n", ret2);
		}
	}
	return ret;
}

/* 
such convert
much danger
need to be fixed
wow 
*/
void convertUnicodeToAnsi(u16* ustr, u8* buf) {
	while(*ustr) {
		u16 wch = *ustr;
		*buf = (char) wch;
		ustr ++;
		buf ++;
	}
	*buf = 0;
}

void convertAnsiToUnicode(u8* ansi, u16* buf) {
	while(*ansi) {
		u8 ch = *ansi;
		*buf = ch;
		ansi ++;
		buf ++;
	}
	*buf = 0;
}

void ustrCat(u16* src, u16* str) {
	while(*src) {
		src ++;
	}
	while(*str) {
		*src = *str;
		str ++;
		src ++;
	}
	*src = 0;
}

u16 testFile []= {'r','a','m',':','/','1',0,0};
u16 ustrRom [] = {'r','o','m',':','/'};
u16 ustrRootPath[200];

u32 userFsTryOpenFileCallback(u32 a1, u16 * fileName, u32 mode) {
	u16 buf[300];
	u32 ret;

	convertUnicodeToAnsi(fileName, (u8*) buf);
	nsDbgPrint("path: %s\n", buf);
	if (memcmp(ustrRom, fileName, sizeof(ustrRom)) == 0) {
		// accessing rom:/ file
		buf[0] = 0;
		ustrCat(buf, ustrRootPath);
		ustrCat(buf, &fileName[5]);
		ret = ((userFsTryOpenFileTypeDef)userFsTryOpenFileHook.callCode)(a1, buf, 1);
		nsDbgPrint("ret: %08x\n", ret);
		if (ret == 0) {
			return ret;
		}
	}
	return ((userFsTryOpenFileTypeDef)userFsTryOpenFileHook.callCode)(a1, fileName, mode);
}

int main() {
	u32 retv;

	initSharedFunc();
	convertAnsiToUnicode((LAYEREDFS_PREFIX), ustrRootPath);
	rtInitHook(&regArchiveHook, (u32) fsRegArchive, (u32) fsRegArchiveCallback);
	rtEnableHook(&regArchiveHook);
	rtInitHook(&userFsTryOpenFileHook, userFsTryOpenFile, (u32)userFsTryOpenFileCallback);
	rtEnableHook(&userFsTryOpenFileHook);
}


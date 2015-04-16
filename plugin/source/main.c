
#include "global.h"
#include "autogen.h"

/* uncomment this to log filename into debugger output */
// #define LOG_FILES

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

typedef u32 (*fsRegArchiveTypeDef)(u8*, u32, u32, u32) ;
typedef u32 (*userFsTryOpenFileTypeDef)(u32, u16*, u32);
typedef u32 (*fsMountArchiveTypeDef)(u32*, u32);


RT_HOOK	regArchiveHook;
RT_HOOK userFsTryOpenFileHook;

u32 fsRegArchiveCallback(u8* path, u32 ptr, u32 isAddOnContent, u32 isAlias) {
	u32 ret, ret2;
	static u32 isFisrt = 1;
	u32 sdmcArchive = 0;

	nsDbgPrint("regArchive: %s, %08x, %08x, %08x\n", path, ptr, isAddOnContent, isAlias);
	ret = ((fsRegArchiveTypeDef)regArchiveHook.callCode)(path, ptr, isAddOnContent, isAlias);
	if (isFisrt) {
		isFisrt = 0;
		
		((fsMountArchiveTypeDef)fsMountArchive)(&sdmcArchive, 9);
		nsDbgPrint("sdmcArchive: %08x\n", sdmcArchive);
		
		if (sdmcArchive) {
			ret2 = ((fsRegArchiveTypeDef)regArchiveHook.callCode)("ram:", sdmcArchive, 0, 0);
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

#ifdef LOG_FILES
	convertUnicodeToAnsi(fileName, (u8*) buf);
	nsDbgPrint("path: %s\n", buf);
#endif

	if (memcmp(ustrRom, fileName, sizeof(ustrRom)) == 0) {
		// accessing rom:/ file
		buf[0] = 0;
		ustrCat(buf, ustrRootPath);
		ustrCat(buf, &fileName[5]);
		ret = ((userFsTryOpenFileTypeDef)userFsTryOpenFileHook.callCode)(a1, buf, 1);
#ifdef LOG_FILES
		nsDbgPrint("ret: %08x\n", ret);
#endif
		if (ret == 0) {
			return ret;
		}
	}
	return ((userFsTryOpenFileTypeDef)userFsTryOpenFileHook.callCode)(a1, fileName, mode);
}
#ifdef ENABLE_LANGEMU
u32 cfgGetRegion = 0;
u32 pCfgHandle = 0;

RT_HOOK cfgReadBlockHook;
RT_HOOK cfgGetRegionHook;

typedef u32 (*cfgReadBlockTypeDef)(u8*, u32, u32);
u32 cfgReadBlockCallback(u8* buf, u32 size, u32 code) {
	u32 ret;
	nsDbgPrint("readblk: %08x, %08x, %08x\n", buf, size, code);
	ret = ((cfgReadBlockTypeDef)cfgReadBlockHook.callCode)(buf, size, code);
	if (code ==  0xA0002) {
		*buf = langCode;
	}	
	return ret;
}


u32 cfgGetRegionCallback(u32* pRegion) {
	*pRegion = regCode;
	nsDbgPrint("region patched\n");
	return 0;
}

u32 findNearestSTMFD(u32 addr) {
	u32 i;
	for (i = 0; i < 1024; i ++) {
		addr -= 4;
		if (*((u16*)(addr + 2)) == 0xe92d) {
			return addr;
		}
	}
	return 0;
}

void findCfgGetRegion() {
	cfgGetRegion = 0;
	u32 addr = 0x00100004;
	u32 lastPage = 0;
	u32 currPage, ret;
	while(1) {
		currPage = rtGetPageOfAddress(addr);
		if (currPage != lastPage) {
			lastPage = currPage;
			ret = rtCheckRemoteMemoryRegionSafeForWrite(getCurrentProcessHandle(), addr, 8);
			if (ret != 0) {
				nsDbgPrint("endaddr: %08x\n", addr);
				return;
			}
		}
		if (*((u32*)addr) == pCfgHandle) {
			if (*((u32*)(addr - 0x2c)) == 0xe3a00802) {
				nsDbgPrint("addr: %08x\n", addr);
				cfgGetRegion = findNearestSTMFD(addr);
				return;
			}
		}
		addr += 4;
	}
}

void findCfgReadBlock() {
	cfgReadBlock = 0;
	u32 addr = 0x00100004;
	u32 lastPage = 0;
	u32 currPage, ret;
	while(1) {
		currPage = rtGetPageOfAddress(addr);
		if (currPage != lastPage) {
			lastPage = currPage;
			ret = rtCheckRemoteMemoryRegionSafeForWrite(getCurrentProcessHandle(), addr, 8);
			if (ret != 0) {
				nsDbgPrint("endaddr: %08x\n", addr);
				return;
			}
		}
		if (*((u32*)addr) == 0x00010082) {
			if (*((u32*)(addr - 4)) == 0xe8bd8010) {
				nsDbgPrint("addr: %08x\n", addr);
				cfgReadBlock = findNearestSTMFD(addr);
				pCfgHandle = *(u32*)(addr + 4);
				return;
			}
		}
		addr += 4;
	}
}
#endif
int main() {
	u32 retv;

	initSharedFunc();
#ifdef ENABLE_LAYEREDFS
	convertAnsiToUnicode((LAYEREDFS_PREFIX), ustrRootPath);
	rtInitHook(&regArchiveHook, (u32) fsRegArchive, (u32) fsRegArchiveCallback);
	rtEnableHook(&regArchiveHook);
	rtInitHook(&userFsTryOpenFileHook, userFsTryOpenFile, (u32)userFsTryOpenFileCallback);
	rtEnableHook(&userFsTryOpenFileHook);
#endif
#ifdef ENABLE_LANGEMU
	findCfgReadBlock();
	nsDbgPrint("cfgreadblock: %08x\n", cfgReadBlock);
	if (cfgReadBlock == 0) {
		return 0;
	}
	rtInitHook(&cfgReadBlockHook, (u32) cfgReadBlock, (u32) cfgReadBlockCallback);
	rtEnableHook(&cfgReadBlockHook);
	findCfgGetRegion();
	nsDbgPrint("cfggetregion: %08x\n", cfgGetRegion);
	if (cfgGetRegion == 0) {
		return 0;
	}

	rtInitHook(&cfgGetRegionHook, (u32) cfgGetRegion, (u32) cfgGetRegionCallback);
	rtEnableHook(&cfgGetRegionHook);
#endif
	return 0;
}


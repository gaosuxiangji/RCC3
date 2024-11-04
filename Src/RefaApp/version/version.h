#ifndef _version_h__
#define _version_h__

#include "autoGenVer.h"


#define	STRINGIZE2(s) #s
#define	STRINGIZE(s) STRINGIZE2(s)

#define	VER_PRODUCTNAME_STR			"RCC"
#define	VER_FILE_DESCRIPTION_STR	VER_PRODUCTNAME_STR

#define	VER_ORIGINAL_FILENAME_STR	"RCC.exe"
#define VER_FILE_VERSION_STR		VERSION_NAME "." VERSION_BUILD "." VERSION_BUILD_DATE
#define	VER_PRODUCT_VERSION_STR		VER_FILE_VERSION_STR
#define	VER_INTERNAL_NAME_STR		VER_ORIGINAL_FILENAME_STR
#define VER_COPYRIGHT_STR			"Copyright (c) " STRINGIZE(VERSION_BUILD_YEAR) " HF Agile Device Co., Ltd.\r\nAll Rights Reserved."
#define VER_COMPANY_NAME_STR		"HF Agile Device Co., Ltd."

#endif // _version_h__

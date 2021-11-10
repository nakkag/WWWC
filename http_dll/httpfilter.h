/**************************************************************************

	WWWC (wwwc.dll)

	httpfilter.h

	Copyright (C) 1996-2003 by Nakashima Tomoaki. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

#ifndef _INC_HTTPFILTER_H
#define _INC_HTTPFILTER_H

/**************************************************************************
	Struct
**************************************************************************/

typedef struct _FILTER {
	int flag;
	BOOL IgnoreCase;
	BOOL IgnoreError;
	BOOL ErrorEmptyBody;
	BOOL Repeat;
	char *url;
	char *string1;
	char *string2;
} FILTER;


/**************************************************************************
	Function Prototypes
**************************************************************************/

BOOL FilterMatch(char *url);
BOOL FilterCheck(char *url, char *buf, int Size);
BOOL ReadFilterFile(char *cpath);
void FreeFilter(void);

#endif
/* End of source */

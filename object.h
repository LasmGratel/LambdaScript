#ifndef LS_OBJECT_H
#define LS_OBJECT_H

typedef union ls_String
{
	ls_AlignmentHeader;
	struct {
		ls_CommonHeader;
		unsigned int len;
		ls_Hash h;
	} s;
} ls_String;

typedef struct ls_ObjectHeader
{
	ls_CommonHeader;
} ls_ObjectHeader;

typedef union ls_Object
{
	ls_ObjectHeader h;
	ls_String s;
} ls_Object;

#endif

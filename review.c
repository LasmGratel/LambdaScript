#include "ls.h"
#include "common.h"
#include "stream.h"
#include "review.h"
#include "lex.h"
#include "parser.h"
#include "code.h"
#include "object.h"

/* review */
#define HLINE "----------------\n"
#define P_TAB "    "

static void print9(Instruction code)
{
	switch (code >> 7)
	{
	case 0:
		printf(" CONST(%d)", code & ((1 << 7) - 1));
		break;
	case 1:
		printf(" STACK(%d)", code & ((1 << 7) - 1));
		break;
	case 2:
		printf(" UPVAL(%d)", code & ((1 << 7) - 1));
		break;
	case 3:
		switch (code & ((1 << 7) - 1))
		{
		case EXP_NTF_NIL:
			printf(" NIL     ");
			break;
		case EXP_NTF_TRUE:
			printf(" TRUE    ");
			break;
		case EXP_NTF_FALSE:
			printf(" FALSE   ");
			break;
		}
		break;
	}
}
static void print7(Instruction code)
{
	printf(" STACK(%d)", code);
}
static void print_code(Instruction code)
{
	printf("0x%x ", code);
	Instruction opcode = code >> (7 + 9 + 9);
	Instruction a = (code >> (9 + 9)) & ((1 << 7) - 1);
	Instruction b = (code >> (9)) & ((1 << 9) - 1);
	Instruction c = (code)& ((1 << 9) - 1);
	switch (opcode)
	{
	case OP_MOVE:
		printf("MOVE     ");
		print9(b);
		printf(" :=");
		print9(c);
		break;
	case OP_GETTABLE:
		printf("GET TABLE");
		print7(a);
		printf(" :=");
		print9(b);
		printf(" [");
		print9(c);
		printf(" ]");
		break;
	case OP_SETTABLE:
		printf("SET TABLE");
		print9(b);
		printf(" [");
		print9(c);
		printf(" ] :=");
		print7(a);
		break;
	case OP_JUMP:
		if (b == 0 && c == 0)
		{
			printf("CLOSE     %d", a);
		}
		break;
	case OP_CLOSURE:
		printf("CLOSURE  ");
		print9(b);
		printf(" :=");
		printf(" PROTO(%d) ", a);
		break;
	case OP_CALL:
		printf("CALL      %s ", a == 0 ? "SINGLE" : a == 1 ? "LIST" : "MULTI");
		print9(b);
		break;
	case OP_EXPANDFILL:
		printf("ADJUST   ");
		if (a)
			print7(a - 1);
		printf(" ->");
		if (b)
			print7(b - 1);
		break;
	}
}

void lsR_reviewcode(ls_Proto* p)
{
	printf("\nCode Review\n");
	printf(HLINE);

	int i;

	//constants
	printf("Constants:\n");
	for (i = 0; i < p->sizek; ++i)
	{
		switch (p->k[i].tt)
		{
		case LS_OBJ_STRING:
			printf(P_TAB "(%d) %s\n", i, getstr(&p->k[i].v.gc->s));
			break;
		case LS_OBJ_NUMBER:
			printf(P_TAB "(%d) %f\n", i, p->k[i].v.n);
			break;
		default:
			printf(P_TAB "(%d) <unknown>\n", i);
		}
	}
	printf(HLINE);

	//local variables
	printf("Local variables:\n");
	for (i = 0; i < p->sizelocvars; ++i)
	{
		printf(P_TAB "(%d) %s\n", i, getstr(p->locvars[i].varname));
	}
	printf(HLINE);

	//upvals
	printf("Upvalues:\n");
	for (i = 0; i < p->sizeupvalues; ++i)
	{
		printf(P_TAB "(%d) %s\n", i, getstr(p->upvalues[i].name));
	}
	printf(HLINE);

	printf("Instructions:\n");
	for (i = 0; i < p->sizecode; ++i)
	{
		printf(P_TAB);
		print_code(p->code[i]);
		printf("\n");
	}
	printf(HLINE);

	printf("Subfunctions:\n");
	for (i = 0; i < p->sizep; ++i)
	{
		printf(P_TAB "(%d) START_SUBFUNCION\n", i);
		lsR_reviewcode(p->p[i]);
		printf(P_TAB "(%d) END_SUBFUNCTION\n", i);
	}
}

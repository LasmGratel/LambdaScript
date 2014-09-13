//Only used in parser.c

static ls_LocVar* getlocvar(ls_ParserData* pd, int i)
{
	int idx = pd->actvar.arr[pd->pf->firstlocal + i].idx;
	ls_assert(idx < pd->pf->nlocvars);
	return &pd->pf->f->locvars[idx];
}

static void adjustlocalvars(ls_ParserData* pd, int nvars)
{
	pd->pf->nactvar = cast_byte(pd->pf->nactvar + nvars);
	for (; nvars; nvars--) {
		getlocvar(pd, pd->pf->nactvar - nvars)->startpc = pd->pf->pc;
	}
}

static int registerlocalvar(ls_ParserData* pd, ls_String* varname)
{
	ls_Proto* f = pd->pf->f;
	int oldsize = f->sizelocvars;
	lsM_growvector(pd->L, f->locvars, pd->pf->nlocvars, f->sizelocvars,
		ls_LocVar, SHRT_MAX, "local variables");
	while (oldsize < f->sizelocvars) f->locvars[oldsize++].varname = NULL;
	f->locvars[pd->pf->nlocvars].varname = varname;
	return pd->pf->nlocvars++;
}

static void newlocalvar(ls_ParserData* pd, ls_String* name)
{
	int reg = registerlocalvar(pd, name);
	checklimit(pd, pd->actvar.n + 1 - pd->pf->firstlocal, MAXVARS, "local variables");
	lsM_growvector(pd->L, pd->actvar.arr, pd->actvar.n + 1,
		pd->actvar.size, ls_Vardesc, MAX_MEMSIZE, "local variables");
	pd->actvar.arr[pd->actvar.n++].idx = cast(short, reg);
}

static int searchvar(ls_ParserData* pd, ls_String* n)
{
	int i;
	for (i = cast_int(pd->pf->nactvar) - 1; i >= 0; i--)
	{
		if (lsS_equal(n, getlocvar(pd, i)->varname))
			return i;
	}
	return -1;  /* not found */
}

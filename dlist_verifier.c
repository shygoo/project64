#include <stdio.h>
#include <strings.h>

#define _LANGUAGE_C
#define gsUnknown(w0, w1) {(w0), (w1)}
#define _SHIFTL(v, s, w) (((unsigned int)(v) & ((1 << (w)) - 1)) << (s))
typedef unsigned int u32;

#include "gbi_version_def.inc.c"
#include "gbi.h"
#include "gbi_redefs.h"

Gfx dlistMacros[] = {
#include "dlist_macros.inc.c"
};

Gfx dlistRaw[] = {
#include "dlist_raw.inc.c"
};

int main(void)
{
	int dlistOk = 1;
	
	int rawLength = sizeof(dlistRaw) / sizeof(Gfx);
	int macroLength = sizeof(dlistRaw) / sizeof(Gfx);
	
	for(int i = 0; i < rawLength; i++)
	{
		if(memcmp(&dlistRaw[i], &dlistMacros[i], sizeof(Gfx)) != 0)
		{
			printf("#%d: generated: %08X %08X, actual: %08X %08X\n",
				i, dlistMacros[i].words.w0, dlistMacros[i].words.w1, dlistRaw[i].words.w0, dlistRaw[i].words.w1);
			return 1;
		}
	}

	return 0;
}
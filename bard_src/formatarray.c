#include "formatarray.h"
#include "logger.h"

void formatarray_kill(struct FormatArray* fmtArray) {
	uint8_t* index = malloc((fmtArray->longestKey+1) * sizeof(uint8_t)); //TODO: WHAT THE HELL C?
	index[0] = '\0';
	char** val;
	JSLF(val, fmtArray->array, index);
	while(val != NULL) {
		if(*val != NULL)
			free(*val);
		JSLN(val, fmtArray->array, index);
	}
	free(index);
	int bytes;
	JSLFA(bytes, fmtArray->array);
}
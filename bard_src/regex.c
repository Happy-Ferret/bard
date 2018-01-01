#define PCRE2_CODE_UNIT_WIDTH 8

#include "regex.h"
#include <pcre2.h>
#include <string.h>
#include "logger.h"
#include "myerror.h"

void regex_init(struct Regex* regex) {
	regex->regexCache = NULL;
	regex->maxLen = 0;
}

void regex_kill(struct Regex* regex) {
	PWord_t val;
	uint8_t index[regex->maxLen];
	index[0] = '\0';
	JSLF(val, regex->regexCache, index);
	while(val != NULL) {
		pcre2_code_free((pcre2_code*)*val);
		JSLN(val, regex->regexCache, index);
	}
	int bytes;
	JSLFA(bytes, regex->regexCache);
	log_write(LEVEL_INFO, "Regex cache was %d bytes long", bytes);
}

struct compileRegexData {
	Pvoid_t* arr;
	size_t* maxLen;
};
bool compileRegex(void* elem, void* userdata) {
	struct Unit* unit = (struct Unit*)elem;
	struct compileRegexData* data = (struct compileRegexData*)userdata;

	int errno;
	PCRE2_SIZE erroroffset;

	pcre2_code* re = pcre2_compile((PCRE2_SPTR)unit->regex,
			PCRE2_ZERO_TERMINATED,
			0,
			&errno,
			&erroroffset,
			NULL);
	PWord_t val;
	JSLI(val, *data->arr, (uint8_t*)unit->name);
	size_t unitNameLen = strlen(unit->name) + 1;
	if(unitNameLen > *(data->maxLen))
		*(data->maxLen) = unitNameLen;

	if(val == PJERR)
		THROW_NEW(false, "Allocation of compiled regex for %s failed", unit->name);
	*val = (unsigned long)re;

	return true;
}

bool regex_compile(struct Regex* regex, struct Units* units) {
	struct compileRegexData data = {
		&regex->regexCache,
		&regex->maxLen,
	};
	vector_foreach(&units->left, compileRegex, &data);
	PROP_THROW(false, "While compiling left side");
	vector_foreach(&units->center, compileRegex, &data);
	PROP_THROW(false, "While compiling center");
	vector_foreach(&units->right, compileRegex, &data);
	PROP_THROW(false, "While compiling right side");
	return true;
}

bool regex_match(struct Regex* regex, struct Unit* unit, char* string, struct FormatArray* array) {
	pcre2_code** val;
	JSLG(val,  regex->regexCache, (uint8_t*)unit->name);
	if(val == NULL)
		THROW_NEW(false, "Tried to match a regex that wasn't compiled for unit %s", unit->name);

	strcpy(array->name, "regex");

	//The first index is always the complete output
	{
		char** val;
		JSLI(val, array->array, (uint8_t*)"1");
		*val = malloc((strlen(string) + 1) * sizeof(char));
		strcpy(*val, string);
	}
	array->longestKey = 5; //TODO: DONT

	if(*val == NULL)
		return true;


	pcre2_match_data* match_data = pcre2_match_data_create_from_pattern(*val, NULL);
	int rc = pcre2_match(*val,
			(PCRE2_SPTR)string,
			strlen(string),
			0,
			0,
			match_data,
			NULL);

	if (rc < 0)
	{
		switch(rc)
		{
			case PCRE2_ERROR_NOMATCH:
				log_write(LEVEL_ERROR, "No match in %s", unit->name);
				break;
			default:
				THROW_NEW(false, "PCRE2 matching error (%d)", rc);
		}
		pcre2_match_data_free(match_data);   /* Release memory used for the match */
		return 1;
	}
	PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(match_data);


	char num[5]; //I don't think anyone will ever match more than 9999 things in bard
	for (int i = 0; i < rc; i++)
	{
		char* substring_start = string + ovector[2*i];
		size_t substring_length = ovector[2*i+1] - ovector[2*i];
		snprintf(num, sizeof(num), "%d", i+2);

		size_t numLen = strlen(num);
		if(numLen > array->longestKey)
			array->longestKey = numLen;

		uint8_t** val;
		JSLI(val, array->array, (uint8_t*)num);
		*val = malloc(substring_length+1);
		strncpy((char*)*val, substring_start, substring_length);
		(*val)[substring_length] = '\0';
		log_write(LEVEL_INFO, "regex group %s is %s", num, *val);
	}
	array->longestKey = 5;

	//TODO: NAMED REGEX MATCHING

	pcre2_match_data_free(match_data);   /* Release memory used for the match */
	return true;
}
/****************************************************************************
 *  Copyright (C) 2010 Reed A. Cartwright, PhD <reed@scit.us>               *
 ****************************************************************************/

#include <dawg/pile_parse.h>

#include <iostream>
#include <fstream>

using namespace dawg;

bool pile::parse_file(pile& p, const char *cs) {
	bool ret;
	if(cs == NULL || strcmp(cs, "")==0 || strcmp(cs, "-")==0) {
		ret = p.parse_stream(std::cin);
	} else {
		std::ifstream is(cs);
		if(!is.is_open())
			return DAWG_ERROR("unable to open input file '" << cs << "'");
		ret = p.parse_stream(is);
	}
	if(!ret)
		return DAWG_ERROR("unable to parse input '" << cs << "'");
	return true;
}

#pragma once
#ifndef DAWG_DETAILS_CONFIG_H
#define DAWG_DETAILS_CONFIG_H

#cmakedefine RANDOM_GEN_HEADER_X @RANDOM_GEN_HEADER_X@  

#ifndef RANDOM_GEN_HEADER
#	define RANDOM_GEN_HEADER RANDOM_GEN_HEADER_X
#endif

#ifndef PACKAGE_STRING
#cmakedefine PACKAGE_STRING "@PACKAGE_STRING@"
#endif

#endif


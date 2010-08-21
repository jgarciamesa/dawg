#pragma once
#ifndef DAWG_PILE_H
#define DAWG_PILE_H
/****************************************************************************
 *  Copyright (C) 2009 Reed A. Cartwright, PhD <reed@scit.us>               *
 ****************************************************************************/

#include <utility>
#include <string>
#include <vector>
#include <map>

#include <dawg/log.h>
#include <dawg/utils/foreach.h>

#include <boost/algorithm/string/predicate.hpp>

namespace dawg {

struct pile {
	struct section {
		typedef std::vector<std::string> value_type;
		typedef std::map<std::string, value_type> db_type;
		std::string name;
		std::string inherits;
		db_type db;
		
		template<typename T>
		inline void get(const std::string& k, T& r) const;
		template<typename T, typename A>
		inline void get(const std::string& k, std::vector<T,A>& r) const;
		
		inline void read_aliases();
		
	private:
		static inline void conv(const std::string& ss, std::string& r);
		static inline void conv(const std::string& ss, double& r);
		static inline void conv(const std::string& ss, bool& r);
		static inline void conv(const std::string& ss, unsigned int& r);
		
		inline void read_alias(const std::string& a, const std::string& b);
	};
	typedef std::vector<section> data_type;
	data_type data;

	static bool parse_file(pile &p, const char *cs);
	template<typename Iterator>
	bool parse(Iterator first, Iterator last);
	template<typename Char, typename Traits>
	inline bool parse_stream(std::basic_istream<Char, Traits>& is);
	
	pile() {
		data.push_back(section());
		data.back().name = "_initial_";
		data.back().inherits = "_default_";
	}
	
	inline void read_aliases();	
};

template<typename T>
inline void pile::section::get(const std::string& k, T& r) const {
	db_type::const_iterator it;
	if((it = db.find(k)) != db.end() && !it->second.empty()) {
		section::conv(it->second.front(), r);
	}
}

template<typename T, typename A>
inline void pile::section::get(const std::string& k, std::vector<T,A>& r) const {
	db_type::const_iterator it;
	if((it = db.find(k)) != db.end()) {
		T x;
		r.clear();
		foreach(const std::string &ss, it->second) {
			section::conv(ss, x);
			r.push_back(x);
		}
	}
}

inline void pile::section::conv(const std::string& ss, std::string& r) {
	r = ss;
}

inline void pile::section::conv(const std::string& ss, double& r) {
	r = strtod(ss.c_str(), NULL);
}

inline void pile::section::conv(const std::string& ss, unsigned int& r) {
	r = strtoul(ss.c_str(), NULL, 0);
}

// A value is false if it is equal to 0, f, false, off, no, or blank
inline void pile::section::conv(const std::string& ss, bool& r) {
	using boost::algorithm::iequals;
	r = !(ss.empty() || iequals(ss, "false") || iequals(ss, "0") || iequals(ss, "f")
		|| iequals(ss, "off") || iequals(ss, "no"));
}

inline void pile::section::read_alias(const std::string& a, const std::string& b) {
	db_type::const_iterator it;
	// if b exists or a doesn't, stop
	if(db.find(b) != db.end() || (it = db.find(a)) == db.end())
		return;
	db[b] = it->second;
}

inline void pile::section::read_aliases() {
	#define XM(aname, bname) read_alias(_P(aname), _P(bname));
	#include <dawg/details/aliases.xmh>
	#undef XM
}

inline void pile::read_aliases() {
	foreach(section &sec, data) {
		sec.read_aliases();
	}
}

} // namespace dawg

#endif //DAWG_PILE_H

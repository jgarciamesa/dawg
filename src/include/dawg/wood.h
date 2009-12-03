#pragma once
#ifndef DAWG_WOOD_H
#define DAWG_WOOD_H

/****************************************************************************
 *  Copyright (C) 2009 Reed A. Cartwright, PhD <reed@scit.us>               *
 ****************************************************************************/

#include <boost/scoped_ptr.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/fusion/include/void.hpp>
#include <boost/spirit/home/phoenix/core/argument.hpp>
#include <boost/spirit/home/phoenix/stl/algorithm/iteration.hpp>

namespace dawg {

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

template <typename Iterator>
struct newick_grammar : qi::grammar<Iterator, ascii::space_type> {
	//http://evolution.genetics.washington.edu/phylip/newick_doc.html

	newick_grammar() : newick_grammar::base_type(start) {
		using boost::spirit::double_;
		using boost::spirit::lexeme;
		using boost::spirit::char_;
		using ascii::space;
		using boost::spirit::arg_names::_1;
		using boost::spirit::arg_names::_val;
		using boost::phoenix::arg_names::arg1;
		using phoenix::val;
		using phoenix::for_each;
		
		start = node >> ';';
		node =  tip | inode;
		tip = label[std::cout << _1 << std::endl] >> -(':' >> double_);
		inode = '(' >> (node % ',') >> ')' >> -(label || (':' >> double_));
		label %= unquoted | quoted;
		unquoted %= lexeme[+(char_ - (char_("()[]':;,")|space))];
		quoted = lexeme['\'' >>
			*(char_ - '\'')[_val += _1] >> *(char_("\'") >> char_("\'")[_val += "'"] >> *(char_ - '\'')[_val += _1])
			>> '\''];
	}
	
	qi::rule<Iterator, ascii::space_type> start;
	qi::rule<Iterator, ascii::space_type> node;
	qi::rule<Iterator, ascii::space_type> tip;
	qi::rule<Iterator, ascii::space_type> inode;
	qi::rule<Iterator, std::string(), ascii::space_type> label;
	qi::rule<Iterator, std::string(), ascii::space_type> unquoted;
	qi::rule<Iterator, std::string(), ascii::space_type> quoted;
};

// Represents a phylogenetic tree structure
class wood {
public:
	struct node {
		boost::scoped_ptr<node> left, right;
		node *anc;
		std::string label;
		double length;		
	};
	
	boost::scoped_ptr<node> root;
	
	template<typename Iterator>
	bool parse(Iterator first, Iterator last) {
		using ascii::space;
		
		newick_grammar<Iterator> newick_parser;
		
		bool r = qi::phrase_parse(first, last, newick_parser, space);
		if( first != last )
			return false;
		return r;
	}
};

}
#endif //DAWG_WOOD_H


#ifndef SPIRIT_H_
#define SPIRIT_H_

//#define as_blob (as<SPL::blob>())
//#define as_base64 (as<iterator_range<base64Iter> >())

#define STR_(OP,VALUE,DELIM,SKIPPER) (OP[*(VALUE - eoi)])
#define STR_W(OP,VALUE,DELIM,SKIPPER) (no_skip[OP[*(VALUE - eoi)]])
#define STR_S(OP,VALUE,DELIM,SKIPPER) (OP[lexeme[*(VALUE - SKIPPER)]])
#define STR_SW(OP,VALUE,DELIM,SKIPPER) (skip(SKIPPER)[OP[no_skip[*(VALUE - SKIPPER)]] >> eps])
#define STR_D(OP,VALUE,DELIM,SKIPPER) (OP[*(VALUE - DELIM)])
#define STR_DW(OP,VALUE,DELIM,SKIPPER) (no_skip[OP[*(VALUE - DELIM)]])
#define STR_DS(OP,VALUE,DELIM,SKIPPER) (OP[*(VALUE - skip(SKIPPER)[DELIM|eoi])])
#define STR_DSW(OP,VALUE,DELIM,SKIPPER) (skip(SKIPPER)[OP[*(VALUE - skip(SKIPPER)[DELIM|eoi])] >> eps])

#include <streams_boost/preprocessor/cat.hpp>
#include <streams_boost/config/warning_disable.hpp>
#include <streams_boost/fusion/include/adapt_struct.hpp>
#include <streams_boost/fusion/include/std_pair.hpp>
#include <streams_boost/spirit/include/phoenix.hpp>
#include <streams_boost/spirit/include/qi.hpp>
#include <streams_boost/spirit/home/support/container.hpp>
#include <streams_boost/spirit/repository/include/qi_advance.hpp>
#include <streams_boost/spirit/repository/include/qi_kwd.hpp>
#include <streams_boost/spirit/repository/include/qi_keywords.hpp>
#include <streams_boost/type_traits/is_same.hpp>
#include <streams_boost/xpressive/xpressive_dynamic.hpp>
#include <streams_boost/algorithm/string/trim.hpp>
#include <streams_boost/archive/iterators/binary_from_base64.hpp>
#include <streams_boost/archive/iterators/transform_width.hpp>
#include "SPL/Runtime/Function/SPLFunctions.h"
#include "time.h"

namespace algorithm = streams_boost::algorithm;
namespace iterators = streams_boost::archive::iterators;
namespace extension = streams_boost::fusion::extension;
namespace fusion = streams_boost::fusion;
namespace mpl = streams_boost::mpl;
namespace phoenix = streams_boost::phoenix;
namespace ascii = streams_boost::spirit::ascii;
namespace qi = streams_boost::spirit::qi;
namespace repo = streams_boost::spirit::repository::qi;
namespace traits = streams_boost::spirit::traits;
namespace xpressive = streams_boost::xpressive;
namespace math = SPL::Functions::Math;

using ascii::char_; using ascii::cntrl; using ascii::punct; using ascii::space;
using fusion::at_c; using qi::locals; using qi::_pass; using qi::_val;
using iterators::binary_from_base64; using iterators::transform_width;
using phoenix::arg_names::arg1; using phoenix::bind; using phoenix::function; using phoenix::construct; using phoenix::ref; using phoenix::val;
using qi::alnum; using qi::alpha; using qi::blank; using qi::string; using qi::symbols;
using qi::bin; using qi::hex; using qi::inf; using qi::oct;
using qi::float_; using qi::double_; using qi::long_double;
using qi::short_; using qi::int_; using qi::long_;
using qi::ushort_; using qi::uint_; using qi::ulong_;
using qi::byte_; using qi::word; using qi::dword; using qi::qword;
using qi::eoi; using qi::eol; using qi::eps; using qi::lit;
using qi::debug; using qi::fail; using qi::on_error;
using qi::as; using qi::as_string; using qi::attr; using qi::attr_cast; using repo::advance; using repo::kwd;
using qi::lexeme; using qi::no_skip; using qi::omit; using qi::raw; using qi::repeat; using qi::skip;
using streams_boost::enable_if; using streams_boost::is_base_of; using streams_boost::iterator_range;
using xpressive::regex_search; using xpressive::cregex;
using namespace qi::labels;

typedef const char* charPtr;
typedef iterator_range<charPtr>::const_iterator (iterator_range<charPtr>::*IterType)(void) const;
typedef transform_width<binary_from_base64<charPtr>, 8, 6> base64Iter;
typedef SPL::timestamp ts;

const std::string dq = "\"";

namespace ext {

	struct dummy_ {
		qi::unused_type unused;
	};

	struct boolean_ : qi::symbols<char, bool> {
		boolean_() {
			add
				("T", true)
				("F", false)
				("t", true)
				("f", false)
				("TRUE", true)
				("FALSE", false)
				("true", true)
				("false", false)
				("1", true)
				("0", false);
		}
	};

	struct error_handler_ {
		template <typename, typename, typename>
		struct result { typedef void type; };

		template <typename Iterator>
		void operator()(qi::info const& what, Iterator err_pos, Iterator last) const {
			std::cout
				<< "Error! Expecting "
				<< what                         // what failed?
				<< " here: \""
				<< std::string(err_pos, last)   // iterators to error-pos, end
				<< "\""
				<< std::endl;
		}
	};

	STREAMS_BOOST_SPIRIT_TERMINAL_EX(base64);

	template <typename Subject>
	struct base64_parser : qi::unary_parser<base64_parser<Subject> > {
		typedef Subject subject_type;

		template <typename Context, typename Iterator>
		struct attribute {
//			typedef streams_boost::variant<std::string,SPL::blob> type;
			typedef typename traits::attribute_of<Subject, Context, Iterator>::type type;
		};

		base64_parser(Subject const& subject) : subject(subject) {}

		template <typename Iterator, typename Context, typename Skipper, typename Attribute>
		bool parse(Iterator& first, Iterator const& last, Context& context, Skipper const& skipper, Attribute& attr) const {
			Iterator rangeFirst, rangeLast;
			rangeFirst = rangeLast = first;

			subject.parse(rangeLast, last, context, skipper, qi::unused);

		    try {
				const iterator_range<base64Iter> base64IterRange(trim_right_copy_if(iterator_range<Iterator>(rangeFirst,rangeLast), algorithm::is_any_of("=")));
				const std::string base64Decoded(base64IterRange.begin(), base64IterRange.end());
				setAttr(attr, base64Decoded);
				first = rangeLast;

			} catch (...) {
				return false;
			}

			return true;
		}

		static void setAttr(std::string & attr, std::string const& base64Decoded) {
//			traits::assign_to(streams_boost::variant<std::string,SPL::blob>(base64Decoded), attr);
			traits::assign_to(base64Decoded, attr);
		}

		static void setAttr(SPL::blob & attr, std::string const& base64Decoded) {
//			traits::assign_to(streams_boost::variant<std::string,SPL::blob>(SPL::blob(reinterpret_cast<const unsigned char*>(base64Decoded.data())), base64Decoded.size()), attr);
			traits::assign_to(SPL::blob(reinterpret_cast<const unsigned char*>(base64Decoded.data()), base64Decoded.size()), attr);
		}

		template <typename Context>
		streams_boost::spirit::info what(Context&) const {
			return streams_boost::spirit::info("base64");
		}

		Subject subject;
	};


	STREAMS_BOOST_SPIRIT_TERMINAL_EX(reparse);

	template <typename Subject, typename RangeSkipper>
	struct reparse_parser : qi::unary_parser<reparse_parser<Subject, RangeSkipper> > {
		typedef Subject subject_type;

		template <typename Context, typename Iterator>
//		struct attribute : traits::attribute_of<Subject, Context, Iterator> {};
		struct attribute {
			typedef typename traits::attribute_of<Subject, Context, Iterator>::type type;
		};

		reparse_parser(Subject const& subject, RangeSkipper const& rangeSkipper) : subject(subject),  rangeSkipper(rangeSkipper) {}

		template <typename Iterator, typename Context, typename Skipper, typename Attribute>
		bool parse(Iterator& first, Iterator const& last, Context& context, Skipper const& skipper, Attribute& attr) const {
			Iterator rangeFirst, rangeLast;
			rangeFirst = rangeLast = first;

			qi::skip_over(rangeLast, last, rangeSkipper);
			if( !subject.parse(rangeFirst, rangeLast, context, skipper, attr)) {
				return false;
			}

			first = rangeLast;
			return true;
		}

		template <typename Context>
		streams_boost::spirit::info what(Context&) const {
			return streams_boost::spirit::info("reparse");
		}

		Subject subject;
		RangeSkipper rangeSkipper;
	};


	STREAMS_BOOST_SPIRIT_TERMINAL_EX(reparse2);

	template <typename Subject, typename RangeSkipper>
	struct reparse2_parser : qi::unary_parser<reparse2_parser<Subject, RangeSkipper> > {
		typedef Subject subject_type;

		template <typename Context, typename Iterator>
		struct attribute {
			typedef typename traits::attribute_of<Subject, Context, Iterator>::type type;
		};

		reparse2_parser(Subject const& subject, RangeSkipper const& rangeSkipper) : subject(subject),  rangeSkipper(rangeSkipper) {}

		template <typename Iterator, typename Context, typename Skipper, typename Attribute>
		bool parse(Iterator& first, Iterator const& last, Context& context, Skipper const& skipper, Attribute& attr) const {
			Iterator rangeFirst = first;

			std::reverse_iterator<Iterator> rrangeFirst(last);
			std::reverse_iterator<Iterator> rrangeLast(first);

			qi::skip_over(rrangeFirst, rrangeLast, rangeSkipper);

			Iterator rangeLast = (++rrangeFirst).base();
			if( !subject.parse(rangeFirst, rangeLast, context, skipper, attr)) {
				return false;
			}

			first = rangeLast;
			return true;
		}

		template <typename Context>
		streams_boost::spirit::info what(Context&) const {
			return streams_boost::spirit::info("reparse2");
		}

		Subject subject;
		RangeSkipper rangeSkipper;
	};


	STREAMS_BOOST_SPIRIT_TERMINAL_EX(reverse);

	template <typename Subject, typename RangeSkipper, typename ReverseSkipper>
	struct reverse_parser : qi::unary_parser<reverse_parser<Subject, RangeSkipper, ReverseSkipper> > {
		typedef Subject subject_type;

		template <typename Context, typename Iterator>
		struct attribute {
			typedef typename traits::attribute_of<Subject, Context, Iterator>::type type;
		};

		reverse_parser(Subject const& subject, RangeSkipper const& rangeSkipper, ReverseSkipper const& reverseSkipper) : subject(subject), rangeSkipper(rangeSkipper), reverseSkipper(reverseSkipper) {}

		template <typename Iterator, typename Context, typename Skipper, typename Attribute>
		bool parse(Iterator& first, Iterator const& last, Context& context, Skipper const& skipper, Attribute& attr) const {
			Iterator rangeFirst, rangeLast;
			rangeFirst = rangeLast = first;
			std::reverse_iterator<Iterator> rrangeLast(first);

			qi::skip_over(rangeLast, last, rangeSkipper);
			Iterator skippedRangeLast = rangeLast;

			if(rangeLast != last) {
				std::reverse_iterator<Iterator> rrangeFirst(rangeLast);
				qi::skip_over(rrangeFirst, rrangeLast, reverseSkipper);
				skippedRangeLast = &(*rrangeFirst);
			}

			if( !subject.parse(rangeFirst, skippedRangeLast, context, skipper, attr)) {
				return false;
			}

			first = skippedRangeLast;
			return true;
		}

		template <typename Context>
		streams_boost::spirit::info what(Context&) const {
			return streams_boost::spirit::info("reverse");
		}

		Subject subject;
		RangeSkipper rangeSkipper;
		ReverseSkipper reverseSkipper;
	};


	STREAMS_BOOST_SPIRIT_TERMINAL_EX(timestampFMT);

    struct timestampFMT_parser : qi::primitive_parser<timestampFMT_parser> {
        template <typename Context, typename Iterator>
        struct attribute {
            typedef SPL::timestamp type;
        };

        timestampFMT_parser(const char* value) : value_(value) {}

        template <typename Iterator, typename Context, typename Skipper, typename Attribute>
        bool parse(Iterator& first, Iterator const& last, Context&, Skipper const& skipper, Attribute& attr) const {
        	qi::skip_over(first, last, skipper);

    		struct tm parsedTm = {};
    		const char* tsParsed = strptime(reinterpret_cast<const char*>(first), value_, &parsedTm);

    		if(tsParsed) {
    			first = reinterpret_cast<charPtr>(tsParsed);
    			// Set current year if no year found in ts
    			if(parsedTm.tm_year == 0) {
    				time_t currTime = time(NULL);
    				struct tm *currTm = localtime(&currTime);
    				parsedTm.tm_year = currTm->tm_year;
    			}
    			parsedTm.tm_isdst = -1;
				time_t rawtime = mktime(&parsedTm);
				if(rawtime >= 0) {
					traits::assign_to(SPL::timestamp(rawtime,0,0), attr);
					return true;
				}
    		}

			return false;
        }

		template <typename Context>
		streams_boost::spirit::info what(Context&) const {
			return streams_boost::spirit::info("timestampFMT");
		}

        const char* value_;
	};
}


namespace streams_boost { namespace spirit {
    template <>
	struct use_directive<qi::domain, ext::tag::base64> : mpl::true_ {};

//    template <>
//	struct use_lazy_directive<qi::domain, ext::tag::base64, 1> : mpl::true_ {};


    template <typename RangeSkipper>
	struct use_directive<qi::domain, terminal_ex<ext::tag::reparse, fusion::vector1<RangeSkipper> > > : mpl::true_ {};

    template <>
	struct use_lazy_directive<qi::domain, ext::tag::reparse, 1> : mpl::true_ {};


    template <typename RangeSkipper>
	struct use_directive<qi::domain, terminal_ex<ext::tag::reparse2, fusion::vector1<RangeSkipper> > > : mpl::true_ {};

    template <>
	struct use_lazy_directive<qi::domain, ext::tag::reparse2, 1> : mpl::true_ {};


    template <typename RangeSkipper, typename ReverseSkipper>
	struct use_directive<qi::domain, terminal_ex<ext::tag::reverse, fusion::vector2<RangeSkipper, ReverseSkipper> > > : mpl::true_ {};

    template <>
	struct use_lazy_directive<qi::domain, ext::tag::reverse, 2> : mpl::true_ {};


    template <>
    struct use_terminal<qi::domain, terminal_ex<ext::tag::timestampFMT, fusion::vector1<const char*> > > : mpl::true_ {};

    template <>
    struct use_lazy_terminal<qi::domain, ext::tag::timestampFMT, 1> : mpl::true_ {};

}}

namespace streams_boost { namespace spirit { namespace qi {

	template <typename Subject, typename Modifiers>
    struct make_directive<ext::tag::base64, Subject, Modifiers> {
        typedef ext::base64_parser<Subject> result_type;

        result_type operator()(unused_type, Subject const& subject, unused_type) const {
            return result_type(subject);
        }
    };


	template <typename RangeSkipper, typename Subject, typename Modifiers>
    struct make_directive<terminal_ex<ext::tag::reparse, fusion::vector1<RangeSkipper> >, Subject, Modifiers> {
        typedef typename result_of::compile<qi::domain, RangeSkipper, Modifiers>::type rangeSkipper_type;
        typedef ext::reparse_parser<Subject, rangeSkipper_type> result_type;

        template <typename Terminal>
        result_type operator()(Terminal const& term, Subject const& subject, Modifiers const& modifiers) const {
            return result_type(subject, compile<qi::domain>(fusion::at_c<0>(term.args), modifiers));
        }
    };


	template <typename RangeSkipper, typename Subject, typename Modifiers>
    struct make_directive<terminal_ex<ext::tag::reparse2, fusion::vector1<RangeSkipper> >, Subject, Modifiers> {
        typedef typename result_of::compile<qi::domain, RangeSkipper, Modifiers>::type rangeSkipper_type;
        typedef ext::reparse2_parser<Subject, rangeSkipper_type> result_type;

        template <typename Terminal>
        result_type operator()(Terminal const& term, Subject const& subject, Modifiers const& modifiers) const {
            return result_type(subject, compile<qi::domain>(fusion::at_c<0>(term.args), modifiers));
        }
    };


	template <typename RangeSkipper, typename ReverseSkipper, typename Subject, typename Modifiers>
    struct make_directive<terminal_ex<ext::tag::reverse, fusion::vector2<RangeSkipper, ReverseSkipper> >, Subject, Modifiers> {
        typedef typename result_of::compile<qi::domain, RangeSkipper, Modifiers>::type rangeSkipper_type;
        typedef typename result_of::compile<qi::domain, ReverseSkipper, Modifiers>::type reverseSkipper_type;
        typedef ext::reverse_parser<Subject, rangeSkipper_type, reverseSkipper_type> result_type;

        template <typename Terminal>
        result_type operator()(Terminal const& term, Subject const& subject, Modifiers const& modifiers) const {
            return result_type(subject, compile<qi::domain>(fusion::at_c<0>(term.args), modifiers), compile<qi::domain>(fusion::at_c<1>(term.args), modifiers));
        }
    };


    template <typename Modifiers>
    struct make_primitive<terminal_ex<ext::tag::timestampFMT, fusion::vector1<const char*> >, Modifiers> {
        typedef ext::timestampFMT_parser result_type;

        template <typename Terminal>
        result_type operator()(Terminal const& term, unused_type) const {
            return result_type(fusion::at_c<0>(term.args));
        }
    };

}}}

namespace streams_boost { namespace spirit { namespace traits {

	template <typename Subject>
    struct has_semantic_action<ext::base64_parser<Subject> > : unary_has_semantic_action<Subject> {};

	template <typename Subject, typename RangeSkipper>
    struct has_semantic_action<ext::reparse_parser<Subject, RangeSkipper> > : mpl::or_<has_semantic_action<Subject>, has_semantic_action<RangeSkipper> > {};

	template <typename Subject, typename RangeSkipper>
    struct has_semantic_action<ext::reparse2_parser<Subject, RangeSkipper> > : mpl::or_<has_semantic_action<Subject>, has_semantic_action<RangeSkipper> > {};

	template <typename Subject, typename RangeSkipper, typename ReverseSkipper>
    struct has_semantic_action<ext::reverse_parser<Subject, RangeSkipper, ReverseSkipper> > : mpl::or_<has_semantic_action<Subject>, has_semantic_action<RangeSkipper>, has_semantic_action<ReverseSkipper> > {};

	template <typename Subject, typename Attribute, typename Context, typename Iterator>
	struct handles_container<ext::base64_parser<Subject>, Attribute, Context, Iterator>
	  : unary_handles_container<Subject, Attribute, Context, Iterator> {};

	template <typename Subject, typename RangeSkipper, typename Attribute, typename Context, typename Iterator>
	struct handles_container<ext::reparse_parser<Subject, RangeSkipper>, Attribute, Context, Iterator>
	  : unary_handles_container<Subject, Attribute, Context, Iterator> {};

	template <typename Subject, typename RangeSkipper, typename Attribute, typename Context, typename Iterator>
	struct handles_container<ext::reparse2_parser<Subject, RangeSkipper>, Attribute, Context, Iterator>
	  : unary_handles_container<Subject, Attribute, Context, Iterator> {};

	template <typename Subject, typename RangeSkipper, typename ReverseSkipper, typename Attribute, typename Context, typename Iterator>
	struct handles_container<ext::reverse_parser<Subject, RangeSkipper, ReverseSkipper>, Attribute, Context, Iterator>
	  : unary_handles_container<Subject, Attribute, Context, Iterator> {};


	template <>
	struct assign_to_attribute_from_value<float, SPL::decimal32> {
		static void call(float& val, SPL::decimal32& attr) {
			attr = SPL::spl_cast<SPL::decimal32, float>::cast(val);
		}
	};

	template <>
	struct assign_to_attribute_from_value<double, SPL::decimal64> {
		static void call(double& val, SPL::decimal64& attr) {
			attr = SPL::spl_cast<SPL::decimal64, double>::cast(val);
		}
	};

	template <>
	struct assign_to_attribute_from_value<long double, SPL::decimal128> {
		static void call(long double& val, SPL::decimal128& attr) {
			attr = SPL::spl_cast<SPL::decimal128, long double>::cast(1);
		}
	};

//	template <>
//	struct assign_to_attribute_from_value<std::basic_string<char>, SPL::blob> {
//		static void call(std::basic_string<char> const& val, SPL::blob& attr) {
//			attr = SPL::blob(reinterpret_cast<const unsigned char*>(val.data()), val.length());
//		}
//	};

	template <typename Iterator>
	struct assign_to_attribute_from_iterators<SPL::blob, Iterator> {
	    static void call(Iterator const& first, Iterator const& last, SPL::blob & attr) {
			attr = SPL::blob(reinterpret_cast<const unsigned char*>(first), last - first);
	    }
	};

//	template <typename Iterator, typename Enum>
//	struct assign_to_attribute_from_iterators<Enum, Iterator, typename enable_if< typename is_base_of<SPL::Enum, Enum>::value>::type> {
//	    static void call(Iterator const& first, Iterator const& last, Enum & attr) {
//	    	STREAMS_BOOST_STATIC_ASSERT((is_base_of<Iterator, SPL::Enum>::value));
//			attr = std::string(first,last);
//	    }
//	};

	template <typename Iterator>
	struct assign_to_attribute_from_iterators<SPL::rstring, Iterator> {
	    static void call(Iterator const& first, Iterator const& last, SPL::rstring & attr) {
			attr = SPL::rstring(first,last);
	    }
	};

	template <typename Iterator, int N>
	struct assign_to_attribute_from_iterators<SPL::bstring<N>, Iterator> {
	    static void call(Iterator const& first, Iterator const& last, SPL::bstring<N> & attr) {
			attr = SPL::bstring<N>(reinterpret_cast<const char*>(first), last - first);
	    }
	};

	template <typename Iterator>
	struct assign_to_attribute_from_iterators<SPL::ustring, Iterator> {
	    static void call(Iterator const& first, Iterator const& last, SPL::ustring & attr) {
			attr = SPL::spl_cast<SPL::ustring, SPL::rstring>::cast(SPL::rstring(first,last));
	    }
	};


	template<>
	struct is_container<SPL::blob> : mpl::true_ {};

	template<>
    struct container_value<SPL::blob> {
		typedef unsigned char type;
	};

	template<>
    struct push_back_container<SPL::blob, unsigned char> {
        static bool call(SPL::blob& b, unsigned char val) {
            b.append(&val, 1);
            return true;
        }
    };

    template <typename K, typename V, int N>
	struct is_container<SPL::bmap<K,V,N> > : mpl::true_ {};

	template <typename K, typename V, int N>
    struct container_value<SPL::bmap<K,V,N> > : mpl::identity<std::pair<K,V> > {};

	template <typename K, typename V, typename P, int N>
    struct push_back_container<SPL::bmap<K,V,N>, P> {
        static bool call(SPL::bmap<K,V,N>& bm, P const& val) {
            bm.insert(val);
            return true;
        }
    };

	template <typename T>
	struct is_container<SPL::set<T> > : mpl::true_ {};

	template <typename T>
    struct container_value<SPL::set<T> > : mpl::identity<T> {};

	template <typename T>
    struct push_back_container<SPL::set<T>, T> {
        static bool call(SPL::set<T>& s, T const& val) {
            s.add(val);
            return true;
        }
    };

	template <typename T, int N>
	struct is_container<SPL::bset<T,N> > : mpl::true_ {};

	template <typename T, int N>
    struct container_value<SPL::bset<T,N> > : mpl::identity<T> {};

	template <typename T, int N>
    struct push_back_container<SPL::bset<T,N>, T> {
        static bool call(SPL::bset<T,N>& bs, T const& val) {
            bs.add(val);
            return true;
        }
    };

//	template <typename Attr>
//	struct transform_attribute<Attr, iterator_range<charPtr>, qi::domain>
//	{
//		typedef iterator_range<charPtr> type;
//		typedef iterator_range<charPtr> const& const_type;
//		typedef Attr & typeToCast;
//
//		static void setData(std::string & val, std::string const& data) { val = data; }
//		static void setData(SPL::blob & val, std::string const& data) {
//			size_t size = data.size();
//			val.setData(reinterpret_cast<const unsigned char*>(data.data()), size);
//		}
//
//		static type pre(typeToCast d) { return type(); }
//		static void post(typeToCast val, const_type attr) {
//			const iterator_range<base64Iter> base64IterRange(trim_right_copy_if(attr, is_any_of("=")));
//			const std::string base64Decoded(base64IterRange.begin(), base64IterRange.end());
//			setData(val, base64Decoded);
//		}
//		static void fail(typeToCast) {}
//	};
}}}

#endif /* SPIRIT_H_ */

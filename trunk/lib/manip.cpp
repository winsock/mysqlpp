/***********************************************************************
 manip.cpp - Implements MySQL++'s various quoting/escaping stream
	manipulators.

 Copyright (c) 1998 by Kevin Atkinson, (c) 1999, 2000 and 2001 by
 MySQL AB, and (c) 2004, 2005 by Educational Technology Resources, Inc.
 Others may also hold copyrights on code in this file.  See the CREDITS
 file in the top directory of the distribution for details.

 This file is part of MySQL++.

 MySQL++ is free software; you can redistribute it and/or modify it
 under the terms of the GNU Lesser General Public License as published
 by the Free Software Foundation; either version 2.1 of the License, or
 (at your option) any later version.

 MySQL++ is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with MySQL++; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 USA
***********************************************************************/

#include "manip.h"

#include "query.h"

using namespace std;

// Manipulator stuff is _always_ in namespace mysqlpp.
namespace mysqlpp {

/// \brief Set to true if you want to suppress automatic quoting
///
/// Works only for ColData inserted into C++ streams.

bool dont_quote_auto = false;


/// \brief Inserts a SQLString into a stream, quoted and escaped.
///
/// If in.is_string is set and in.dont_escape is \e not set, the string
/// is quoted and escaped.
///
/// If both in.is_string and in.dont_escape are set, the string is
/// quoted but not escaped.
///
/// If in.is_string is not set, the data is inserted as-is.  This is
/// the case when you initialize SQLString with one of the constructors
/// taking an integral type, for instance.

SQLQueryParms& operator <<(quote_type2 p, SQLString& in)
{
	if (in.is_string) {
		SQLString in2('\'');
		if (in.dont_escape) {
			in2 += in;
			in2 += '\'';
			in2.processed = true;
			return *p.qparms << in2;
		}
		else {
			char* s = new char[in.length() * 2 + 1];
			size_t len = mysql_escape_string(s, in.data(), in.length());
			in2.append(s, len);
			in2 += '\'';
			in2.processed = true;
			*p.qparms << in2;
			delete[] s;
			return *p.qparms;
		}
	}
	else {
		in.processed = true;
		return *p.qparms << in;
	}
}


/// \brief Inserts a C++ string into a stream, quoted and escaped
///
/// Because std::string lacks the type information we need, the string
/// is both quoted and escaped, always.

template <>
ostream& operator <<(quote_type1 o, const string& in)
{
	char* s = new char[in.length() * 2 + 1];
	size_t len = mysql_escape_string(s, in.data(), in.length());

	o.ostr->write("'", 1);
	o.ostr->write(s, len);
	o.ostr->write("'", 1);

	delete[] s;
	return *o.ostr;
}


/// \brief Inserts a C string into a stream, quoted and escaped
///
/// Because C strings lack the type information we need, the string
/// is both quoted and escaped, always.

template <>
ostream& operator <<(quote_type1 o, const char* const& in)
{
	size_t len = strlen(in);
	char* s = new char[len * 2 + 1];
	len = mysql_escape_string(s, in, len);

	o.ostr->write("'", 1);
	o.ostr->write(s, len);
	o.ostr->write("'", 1);

	delete[] s;
	return *o.ostr;
}


/// \brief Inserts a ColData with const string into a stream, quoted and
/// escaped
///
/// Because ColData was designed to contain MySQL type data, we may
/// choose not to actually quote or escape the data, if it is not
/// needed.

template <>
ostream& operator <<(quote_type1 o, const ColData& in)
{
	if (in.escape_q()) {
		char* s = new char[in.length() * 2 + 1];
		size_t len = mysql_escape_string(s, in.data(), in.length());

		if (in.quote_q()) o.ostr->write("'", 1);
		o.ostr->write(s, len);
		if (in.quote_q()) o.ostr->write("'", 1);

		delete[] s;
	}
	else {
		if (in.quote_q()) o.ostr->write("'", 1);
		o.ostr->write(in.data(), in.length());
		if (in.quote_q()) o.ostr->write("'", 1);
	}

	return *o.ostr;
}


/// \brief Inserts a ColData into a non-Query stream.
///
/// Although we know how to automatically quote and escape ColData
/// objects, we only do that when inserting them into Query streams
/// because this feature is only intended to make it easier to build
/// syntactically-correct SQL queries.  You can force the library to
/// give you quoting and escaping with the quote manipulator:
///
/// \code
/// mysqlpp::ColData cd("...");
/// cout << mysqlpp::quote << cd << endl;
/// \endcode

ostream& operator <<(ostream& o, const ColData& in)
{
	o.write(in.data(), in.length());
	return o;
}


/// \brief Insert a ColData with const string into a SQLQuery
///
/// This operator appears to be a workaround for a weakness in one
/// compiler's implementation of the C++ type system.  See Wishlist for
/// current plan on what to do about this.

Query& operator <<(Query& o, const ColData& in)
{
	if (dont_quote_auto) {
		o.write(in.data(), in.length());
	}
	else if (in.escape_q()) {
		char* s = new char[in.length() * 2 + 1];
		size_t len = mysql_escape_string(s, in.data(), in.length());

		if (in.quote_q()) o.write("'", 1);
		o.write(s, len);
		if (in.quote_q()) o.write("'", 1);

		delete[] s;
	}
	else {
		if (in.quote_q()) o.write("'", 1);
		o.write(in.data(), in.length());
		if (in.quote_q()) o.write("'", 1);
	}

	return o;
}


/// \brief Inserts a SQLString into a stream, quoting it unless it's
/// data that needs no quoting.
///
/// We make the decision to quote the data based on the in.is_string
/// flag.  You can set it yourself, but SQLString's ctors should set
/// it correctly for you.

SQLQueryParms& operator <<(quote_only_type2 p, SQLString& in)
{
	if (in.is_string) {
		SQLString in2;
		in2 = '\'' + in + '\'';
		in2.processed = true;
		return *p.qparms << in2;
	}
	else {
		in.processed = true;
		return *p.qparms << in;
	}
}


/// \brief Inserts a ColData with const string into a stream, quoted
///
/// Because ColData was designed to contain MySQL type data, we may
/// choose not to actually quote the data, if it is not needed.

template <>
ostream& operator <<(quote_only_type1 o, const ColData& in)
{
	if (in.quote_q()) o.ostr->write("'", 1);
	o.ostr->write(in.data(), in.length());
	if (in.quote_q()) o.ostr->write("'", 1);

	return *o.ostr;
}


/// \brief Inserts a SQLString into a stream, double-quoting it (")
/// unless it's data that needs no quoting.
///
/// We make the decision to quote the data based on the in.is_string
/// flag.  You can set it yourself, but SQLString's ctors should set
/// it correctly for you.

SQLQueryParms& operator <<(quote_double_only_type2 p, SQLString& in)
{
	if (in.is_string) {
		SQLString in2;
		in2 = "\"" + in + "\"";
		in2.processed = true;
		return *p.qparms << in2;
	}
	else {
		in.processed = true;
		return *p.qparms << in;
	}
}


/// \brief Inserts a ColData with const string into a stream,
/// double-quoted (")
///
/// Because ColData was designed to contain MySQL type data, we may
/// choose not to actually quote the data, if it is not needed.

template <>
ostream& operator <<(quote_double_only_type1 o, const ColData& in)
{
	if (in.quote_q()) o.ostr->write("'", 1);
	o.ostr->write(in.data(), in.length());
	if (in.quote_q()) o.ostr->write("'", 1);

	return *o.ostr;
}


SQLQueryParms& operator <<(escape_type2 p, SQLString& in)
{
	if (in.is_string && !in.dont_escape) {
		char* s = new char[in.length() * 2 + 1];
		size_t len = mysql_escape_string(s, in.data(), in.length());

		SQLString in2(s, len);
		in2.processed = true;
		*p.qparms << in2;

		delete[] s;
		return *p.qparms;
	}
	else {
		in.processed = true;
		return *p.qparms << in;
	}
}


/// \brief Inserts a C++ string into a stream, escaping special SQL
/// characters
///
/// Because std::string lacks the type information we need, the string
/// is always escaped, even if it doesn't need it.

template <>
std::ostream& operator <<(escape_type1 o, const std::string& in)
{
	char* s = new char[in.length() * 2 + 1];
	size_t len = mysql_escape_string(s, in.data(), in.length());
	o.ostr->write(s, len);
	delete[] s;
	return *o.ostr;
}


/// \brief Inserts a C string into a stream, escaping special SQL
/// characters
///
/// Because C's type system lacks the information we need to second-
/// guess this manipulator, we always run the escaping algorithm on
/// the data, even if it's not needed.

template <>
ostream& operator <<(escape_type1 o, const char* const& in)
{
	size_t len = strlen(in);
	char* s = new char[len * 2 + 1];
	len = mysql_escape_string(s, in, len);
	o.ostr->write(s, len);
	delete[] s;
	return *o.ostr;
}


/// \brief Inserts a ColData with const string into a stream, escaping
/// special SQL characters
///
/// Because ColData was designed to contain MySQL type data, we may
/// choose not to escape the data, if it is not needed.

std::ostream& operator <<(escape_type1 o, const ColData& in)
{
	if (in.escape_q()) {
		char* s = new char[in.length() * 2 + 1];
		size_t len = mysql_escape_string(s, in.data(), in.length());
		o.ostr->write(s, len);
		delete[] s;
	}
	else {
		o.ostr->write(in.data(), in.length());
	}

	return *o.ostr;
}


/// \brief Inserts a SQLString into a stream, with no escaping or
/// quoting.

SQLQueryParms& operator <<(do_nothing_type2 p, SQLString& in)
{
	in.processed = true;
	return *p.qparms << in;
}


/// \brief Inserts a SQLString into a stream, with no escaping or
/// quoting, and without marking the string as having been "processed".

SQLQueryParms& operator <<(ignore_type2 p, SQLString& in)
{
	return *p.qparms << in;
}

} // end namespace mysqlpp


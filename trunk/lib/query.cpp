/***********************************************************************
 query.cpp - Implements the Query class.

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

#include "query.h"

#include "connection.h"

namespace mysqlpp {

Query::Query(const Query& q) :
std::ostream(&sbuffer_),	
OptionalExceptions(q.exceptions()),
Lockable(),
conn_(q.conn_),
success_(q.success_),
errmsg_(q.errmsg_),
def(q.def)
{
}


my_ulonglong Query::affected_rows() const
{
	return conn_->affected_rows();
}


std::string Query::error()
{
	if (errmsg_) {
		return std::string(errmsg_);
	}
	else {
		return conn_->error();
	}
}


bool Query::exec(const std::string& str)
{
	success_ = !mysql_real_query(&conn_->mysql, str.c_str(),
			(unsigned long)str.length());
	if (!success_ && throw_exceptions()) {
		throw BadQuery(conn_->error());
	}
	else {
		return success_;
	}
}


ResNSel Query::execute(const char* str)
{
	success_ = false;
	if (lock()) {
		if (throw_exceptions()) {
			throw BadQuery("lock failed");
		}
		else {
			return ResNSel();
		}
	}

	success_ = !mysql_query(&conn_->mysql, str);
	unlock();
	if (success_) {
		return ResNSel(conn_);
	}
	else {
		if (throw_exceptions()) {
			throw BadQuery(conn_->error());
		}
		else {
			return ResNSel();
		}
	}
}


ResNSel Query::execute(parms& p)
{
	query_reset r = parse_elems_.size() ? DONT_RESET : RESET_QUERY;
	return execute(str(p, r).c_str());
}


std::string Query::info()
{
	return conn_->info();
}


my_ulonglong Query::insert_id()
{
	return conn_->insert_id();
}


bool Query::lock()
{
    return conn_->lock();
}


void Query::parse()
{
	std::string str = "";
	char num[4];
	long int n;
	char option;
	std::string name;
	char *s, *s0;
	s0 = s = preview_char();
	while (*s) {
		if (*s == '%') {
			s++;
			if (*s == '%') {
				str += *s++;
			}
			else if (*s >= '0' && *s <= '9') {
				num[0] = *s;
				s++;
				if (*s >= '0' && *s <= '9') {
					num[1] = *s;
					num[2] = 0;
					s++;
					if (*s >= '0' && *s <= '9') {
						num[2] = *s;
						num[3] = 0;
						s++;
					}
					else {
						num[2] = 0;
					}
				}
				else {
					num[1] = 0;
				}

				n = strtol(num, 0, 10);
				option = ' ';

				if (*s == 'q' || *s == 'Q' || *s == 'r' || *s == 'R') {
					option = *s++;
				}

				if (*s == ':') {
					s++;
					for ( /* */ ; (*s >= 'A' && *s <= 'Z') ||
						 *s == '_' || (*s >= 'a' && *s <= 'z'); s++) {
						name += *s;
					}

					if (*s == ':') {
						s++;
					}

					if (n >= static_cast<long int>(parsed_names_.size())) {
						parsed_names_.insert(parsed_names_.end(),
								static_cast<std::vector<std::string>::size_type>(
										n + 1) - parsed_names_.size(),
								std::string());
					}
					parsed_names_[n] = name;
					parsed_nums_[name] = n;
				}

				parse_elems_.push_back(SQLParseElement(str, option, char(n)));
				str = "";
				name = "";
			}
			else {
				str += '%';
			}
		}
		else {
			str += *s++;
		}
	}

	parse_elems_.push_back(SQLParseElement(str, ' ', -1));
	delete[] s0;
}


SQLString*
Query::pprepare(char option, SQLString& S, bool replace)
{
	if (S.processed) {
		return &S;
	}

	if (option == 'r' || (option == 'q' && S.is_string)) {
		char *s = new char[S.size() * 2 + 1];
		mysql_real_escape_string(&conn_->mysql, s, S.c_str(),
				static_cast<unsigned long>(S.size()));
		SQLString *ss = new SQLString("'");
		*ss += s;
		*ss += "'";
		delete[] s;

		if (replace) {
			S = *ss;
			S.processed = true;
			return &S;
		}
		else {
			return ss;
		}
	}
	else if (option == 'R' || (option == 'Q' && S.is_string)) {
		SQLString *ss = new SQLString("'" + S + "'");

		if (replace) {
			S = *ss;
			S.processed = true;
			return &S;
		}
		else {
			return ss;
		}
	}
	else {
		if (replace) {
			S.processed = true;
		}
		return &S;
	}
}


char* Query::preview_char()
{
	*this << std::ends;
	size_t length = sbuffer_.str().size();
	char* s = new char[length + 1];
	strcpy(s, sbuffer_.str().c_str());
	return s;
}


void Query::proc(SQLQueryParms& p)
{
	sbuffer_.str("");

	char num;
	SQLString* ss;
	SQLQueryParms* c;

	for (std::vector<SQLParseElement>::iterator i = parse_elems_.begin();
			i != parse_elems_.end(); ++i) {
		dynamic_cast<std::ostream&>(*this) << i->before;
		num = i->num;
		if (num != -1) {
			if (num < static_cast<int>(p.size()))
				c = &p;
			else if (num < static_cast<int>(def.size()))
				c = &def;
			else {
				*this << " ERROR";
				throw BadParamCount(
						"Not enough parameters to fill the template.");
			}
			ss = pprepare(i->option, (*c)[num], c->bound());
			dynamic_cast<std::ostream&>(*this) << *ss;
			if (ss != &(*c)[num]) {
				delete ss;
			}
		}
	}
}

void Query::reset()
{
	seekp(0);
	clear();
	sbuffer_.str("");

	parse_elems_.clear();
	def.clear();
}


Result Query::store(const char* str)
{
	success_ = false;

	if (lock()) {
		if (throw_exceptions()) {
			throw BadQuery("lock failed");
		}
		else {
			return Result();
		}
	}

	success_ = !mysql_query(&conn_->mysql, str);
	if (success_) {
		MYSQL_RES* res = mysql_store_result(&conn_->mysql);
		if (res) {
			unlock();
			return Result(res, throw_exceptions());
		}
	}
	unlock();

	// One of the mysql_* calls failed, so decide how we should fail.
	if (throw_exceptions()) {
		throw BadQuery(conn_->error());
	}
	else {
		return Result();
	}
}


Result Query::store(parms& p)
{
	query_reset r = parse_elems_.size() ? DONT_RESET : RESET_QUERY;
	return store(str(p, r).c_str());
}


std::string Query::str(SQLQueryParms& p)
{
	if (!parse_elems_.empty()) {
		proc(p);
	}

	*this << std::ends;

	return sbuffer_.str();
}


std::string Query::str(SQLQueryParms& p, query_reset r)
{
	std::string tmp = str(p);
	if (r == RESET_QUERY) {
		reset();
	}
	return tmp;
}


bool Query::success()
{
	if (success_) {
		return conn_->success();
	}
	else {
		return false;
	}
}


void Query::unlock()
{
	conn_->unlock();
}


ResUse Query::use(const char* str)
{
	success_ = false;
	if (lock()) {
		if (throw_exceptions()) {
			throw BadQuery("lock failed");
		}
		else {
			return ResUse();
		}
	}

	success_ = !mysql_query(&conn_->mysql, str);
	if (success_) {
		MYSQL_RES* res = mysql_use_result(&conn_->mysql);
		if (res) {
			unlock();
			return ResUse(res, conn_, throw_exceptions());
		}
	}
	unlock();

	// One of the mysql_* calls failed, so decide how we should fail.
	if (throw_exceptions()) {
		throw BadQuery(conn_->error());
	}
	else {
		return ResUse();
	}
}


ResUse Query::use(parms& p)
{
	query_reset r = parse_elems_.size() ? DONT_RESET : RESET_QUERY;
	return use(str(p, r).c_str());
}


} // end namespace mysqlpp


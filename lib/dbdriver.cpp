/***********************************************************************
 dbdriver.cpp - Implements the DBDriver class.

 Copyright (c) 2005-2007 by Educational Technology Resources, Inc.
 Others may also hold copyrights on code in this file.  See the
 CREDITS file in the top directory of the distribution for details.

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

#define MYSQLPP_NOT_HEADER
#include "dbdriver.h"

#include "exceptions.h"

#include <sstream>

// An argument was added to mysql_shutdown() in MySQL 4.1.3 and 5.0.1.
#if ((MYSQL_VERSION_ID >= 40103) && (MYSQL_VERSION_ID <= 49999)) || (MYSQL_VERSION_ID >= 50001)
#	define SHUTDOWN_ARG ,SHUTDOWN_DEFAULT
#else
#	define SHUTDOWN_ARG
#endif

using namespace std;

namespace mysqlpp {

DBDriver::DBDriver() :
is_connected_(false)
{
	mysql_init(&mysql_);
}


DBDriver::DBDriver(const DBDriver& other) :
is_connected_(false)
{
	copy(other);
}


DBDriver::~DBDriver()
{
	if (connected()) {
		disconnect();
	}
}


bool
DBDriver::connect(const char* host, const char* socket_name,
		unsigned int port, const char* db, const char* user,
		const char* password)
{
	// Drop previous connection, if any
	if (connected()) {
		disconnect();
	}

	// Set defaults for connection options.  User can override these
	// by calling set_option() before connect().
	set_option_default(new ReadDefaultFileOption("my"));

	// Establish the connection
	return is_connected_ =
			mysql_real_connect(&mysql_, host, user, password, db,
			port, socket_name, mysql_.client_flag);
}


bool
DBDriver::connect(const MYSQL& other)
{
	// Drop previous connection, if any
	if (connected()) {
		disconnect();
	}

	// Set defaults for connection options.  User can override these
	// by calling set_option() before connect().
	set_option_default(new ReadDefaultFileOption("my"));

	// Establish the connection
	return is_connected_ =
			mysql_real_connect(&mysql_, other.host, other.user,
			other.passwd, other.db, other.port, other.unix_socket,
			other.client_flag);
}


void
DBDriver::copy(const DBDriver& other)
{
	if (other.connected()) {
		connect(other.mysql_);
	}
	else {
		is_connected_ = false;
	}
}


void
DBDriver::disconnect()
{
	mysql_close(&mysql_);
	is_connected_ = false;
}


const char*
DBDriver::enable_ssl(const char* key, const char* cert,
		const char* ca, const char* capath, const char* cipher)
{
#if defined(HAVE_MYSQL_SSL_SET)
	mysql_ssl_set(&mysql_, key, cert, ca, capath, cipher);
	return "";
#else
	return "SSL not enabled in MySQL++";
#endif
}


DBDriver&
DBDriver::operator=(const DBDriver& rhs)
{
	copy(rhs);
	return *this;
}


string
DBDriver::query_info()
{
	const char* i = mysql_info(&mysql_);
	return i ? string(i) : string();
}


bool
DBDriver::set_option(int o, bool arg)
{
	// If we get through this loop and n is 1, only one bit is set in
	// the option value, which is as it should be.
	int n = o;
	while (n && ((n & 1) == 0)) {
		n >>= 1;
	}
	
	if ((n == 1) && 
			(o >= CLIENT_LONG_PASSWORD) && 
			(o <= CLIENT_MULTI_RESULTS)) {
		// Option value seems sane, so go ahead and set/clear the flag
		if (arg) {
			mysql_.client_flag |= o;
		}
		else {
			mysql_.client_flag &= ~o;
		}

		return true;
	}
	else {
		// Option value is outside the range we understand, or caller
		// erroneously passed a value with multiple bits set.
		return false;
	}
}


std::string
DBDriver::set_option(Option* o)
{
	std::ostringstream os;
	switch (o->set(this)) {
		case Option::err_NONE:
			applied_options_.push_back(o);
			break;

		case Option::err_api_limit:
			os << "Database driver version " << client_version() <<
					"doesn't support option '" << typeid(o).name() << "'";
			throw BadOption(os.str(), o); // mandatory throw!

		case Option::err_api_reject:
			os << "Database driver returned an error when setting "
					"option '" << typeid(o).name() << "'";
			break;

		case Option::err_bad_arg:
			os << "Failed to set option '" << typeid(o).name() <<
					"'; bad argument value?";
			break;

		case Option::err_connected:
			os << "Option '" << typeid(o).name() <<
					"' can only be set before connection is established.";
			break;
	}
	return os.str();
}


bool
DBDriver::shutdown()
{
	return mysql_shutdown(&mysql_ SHUTDOWN_ARG);
}


bool
DBDriver::thread_aware() const
{
#if defined(MYSQLPP_PLATFORM_WINDOWS) || defined(HAVE_PTHREAD) || defined(HAVE_SYNCH_H)
	// Okay, good, MySQL++ itself is thread-aware, but only return true
	// if the underlying C API library is also thread-aware.
	return mysql_thread_safe();
#else
	// MySQL++ itself isn't thread-aware, so we don't need to do any
	// further tests.  All pieces must be thread-aware to return true.
	return false;	
#endif
}

} // end namespace mysqlpp


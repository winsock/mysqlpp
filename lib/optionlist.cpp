/***********************************************************************
 optionlist.cpp - Implements the Type class hierarchy and related
 	things.

 Copyright (c) 2007 by Educational Technology Resources, Inc.  Others
 may also hold copyrights on code in this file.  See the CREDITS
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

#define MYSQLPP_NOT_HEADER
#include "optionlist.h"

#include "dbdriver.h"


namespace mysqlpp {

Option::Error
CompressOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_OPT_COMPRESS) ?
				Option::err_NONE : Option::err_api_reject;
}


Option::Error
ConnectTimeoutOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_OPT_CONNECT_TIMEOUT, &arg_) ?
				Option::err_NONE : Option::err_bad_arg;
}


Option::Error
FoundRowsOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(CLIENT_FOUND_ROWS, arg_) ?
				Option::err_NONE : Option::err_bad_arg;
}


Option::Error
GuessConnectionOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 40101
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_OPT_GUESS_CONNECTION) ?
				Option::err_NONE : Option::err_api_reject;
#else
	return Option::err_api_limit;
#endif
}


Option::Error
IgnoreSpaceOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(CLIENT_IGNORE_SPACE, arg_) ?
				Option::err_NONE : Option::err_bad_arg;
}


Option::Error
InitCommandOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_INIT_COMMAND, arg_.c_str()) ?
				Option::err_NONE : Option::err_bad_arg;
}


Option::Error
InteractiveOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(CLIENT_INTERACTIVE, arg_) ?
				Option::err_NONE : Option::err_bad_arg;
}


Option::Error
LocalFilesOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(CLIENT_LOCAL_FILES, arg_) ?
				Option::err_NONE : Option::err_bad_arg;
}


Option::Error
LocalInfileOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_OPT_LOCAL_INFILE, &arg_) ?
				Option::err_NONE : Option::err_bad_arg;
}


Option::Error
MultiResultsOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 40101
	if (dbd->connected()) {
		return dbd->set_option(arg_ ? MYSQL_OPTION_MULTI_STATEMENTS_ON :
				MYSQL_OPTION_MULTI_STATEMENTS_OFF) ?
				Option::err_NONE : Option::err_bad_arg;
	}
	else {
		return dbd->set_option(CLIENT_MULTI_RESULTS, arg_) ?
				Option::err_NONE : Option::err_bad_arg;
	}
#else
	return Option::err_api_limit;
#endif
}


Option::Error
MultiStatementsOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 40101
	if (dbd->connected()) {
		return dbd->set_option(arg_ ? MYSQL_OPTION_MULTI_STATEMENTS_ON :
				MYSQL_OPTION_MULTI_STATEMENTS_OFF) ?
				Option::err_NONE : Option::err_bad_arg;
	}
	else {
		return dbd->set_option(CLIENT_MULTI_STATEMENTS, arg_) ?
				Option::err_NONE : Option::err_bad_arg;
	}
#else
	return Option::err_api_limit;
#endif
}


Option::Error
NamedPipeOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_OPT_NAMED_PIPE) ?
				Option::err_NONE : Option::err_api_reject;
}


Option::Error
NoSchemaOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(CLIENT_NO_SCHEMA, arg_) ?
				Option::err_NONE : Option::err_bad_arg;
}


Option::Error
ProtocolOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_OPT_PROTOCOL, &arg_) ?
				Option::err_NONE : Option::err_bad_arg;
}


Option::Error
ReadDefaultFileOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_READ_DEFAULT_FILE, arg_.c_str()) ?
				Option::err_NONE : Option::err_bad_arg;
}


Option::Error
ReadDefaultGroupOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_READ_DEFAULT_GROUP, arg_.c_str()) ?
				Option::err_NONE : Option::err_bad_arg;
}


Option::Error
ReadTimeoutOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 40101
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_OPT_READ_TIMEOUT, &arg_) ?
				Option::err_NONE : Option::err_bad_arg;
#else
	return Option::err_api_limit;
#endif
}


Option::Error
ReconnectOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 50013
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_OPT_RECONNECT, &arg_) ?
				Option::err_NONE : Option::err_bad_arg;
#else
	return Option::err_api_limit;
#endif
}


Option::Error
ReportDataTruncationOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 50003
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_REPORT_DATA_TRUNCATION, &arg_) ?
				Option::err_NONE : Option::err_bad_arg;
#else
	return Option::err_api_limit;
#endif
}


Option::Error
SecureAuthOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 40101
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_SECURE_AUTH, &arg_) ?
				Option::err_NONE : Option::err_bad_arg;
#else
	return Option::err_api_limit;
#endif
}


Option::Error
SetCharsetDirOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_SET_CHARSET_DIR, arg_.c_str()) ?
				Option::err_NONE : Option::err_bad_arg;
}


Option::Error
SetCharsetNameOption::set(DBDriver* dbd)
{
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_SET_CHARSET_NAME, arg_.c_str()) ?
				Option::err_NONE : Option::err_bad_arg;
}


Option::Error
SetClientIpOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 40101
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_SET_CLIENT_IP, arg_.c_str()) ?
				Option::err_NONE : Option::err_bad_arg;
#else
	return Option::err_api_limit;
#endif
}


Option::Error
SharedMemoryBaseNameOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 40100
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_SHARED_MEMORY_BASE_NAME, arg_.c_str()) ?
				Option::err_NONE : Option::err_bad_arg;
#else
	return Option::err_api_limit;
#endif
}


Option::Error
UseEmbeddedConnectionOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 40101
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_OPT_USE_EMBEDDED_CONNECTION) ?
				Option::err_NONE : Option::err_api_reject;
#else
	return Option::err_api_limit;
#endif
}


Option::Error
UseRemoteConnectionOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 40101
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_OPT_USE_REMOTE_CONNECTION) ?
				Option::err_NONE : Option::err_api_reject;
#else
	return Option::err_api_limit;
#endif
}


Option::Error
WriteTimeoutOption::set(DBDriver* dbd)
{
#if MYSQL_VERSION_ID >= 40101
	return dbd->connected() ? Option::err_connected :
			dbd->set_option(MYSQL_OPT_WRITE_TIMEOUT, &arg_) ?
				Option::err_NONE : Option::err_bad_arg;
#else
	return Option::err_api_limit;
#endif
}


} // end namespace mysqlpp

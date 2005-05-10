/***********************************************************************
 custom3.cpp - Example showing how to update an SQL row using the
	Specialized SQL Structures feature of MySQL++.

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

#include "util.h"

#include <mysql++.h>
#include <custom.h>

#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace mysqlpp;

sql_create_5(stock,
			1, 5,
			string, item,
			longlong, num,
			double, weight,
			double, price,
			Date, sdate)

int
main(int argc, char *argv[])
{
	try {
		Connection con(use_exceptions);
		if (!connect_to_db(argc, argv, con)) {
			return 1;
		}

		Query query = con.query();
		query << "select * from stock where item = \"Nürnberger Brats\"";

		// Is the query was successful?  If not throw a bad query.
		Result res = query.store();
		if (res.empty()) {
			throw BadQuery("UTF-8 bratwurst item not found in "
					"table, run resetdb");
		}

		// Because there should only be one row in the result set, we
		// don't need to use a vector.  Just store the first row
		// directly in "row".  We can do this because one of the
		// constructors for stock takes a Row as a parameter.
		stock row = res[0];

		// Create a copy so that the replace query knows what the
		// original values are.
		stock row2 = row;

		// Change item column to use only 7-bit ASCII, and to
		// deliberately be wider than normal column widths printed by
		// print_stock_table().
		row.item = "Nuerenberger Bratwurst";

		// Form the query to replace the row.  The table name is the
		// name of the struct by default.
		query.update(row2, row);

		// Show the query about to be executed.
		cout << "Query : " << query.preview() << endl;

		// Call execute(), since the query won't return a result set.
		query.execute();

		// Now print the new table
		print_stock_table(query);
	}
	catch (BadQuery& er) {
		// handle any connection or query errors that may come up
		cerr << "Error: " << er.what() << endl;
		return -1;
	}
	catch (BadConversion& er) {
		// handle bad conversions
		cerr << "Error: " << er.what() << "\"." << endl
			<< "retrieved data size: " << er.retrieved
			<< " actual data size: " << er.actual_size << endl;
		return -1;
	}
	catch (exception& er) {
		cerr << "Error: " << er.what() << endl;
		return -1;
	}

	return 0;
}


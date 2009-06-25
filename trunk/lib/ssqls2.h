/// \file ssqls2.h
/// \brief Declares the SsqlsBase class

/***********************************************************************
 Copyright (c) 2009 by Educational Technology Resources, Inc.
 Others may also hold copyrights on code in this file.  See the
 CREDITS.txt file in the top directory of the distribution for details.

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

#if !defined(MYSQLPP_SSQLS2_H)
#define MYSQLPP_SSQLS2_H

#include "common.h"

#include <iostream>

namespace mysqlpp {

#if !defined(DOXYGEN_IGNORE)
// Make Doxygen ignore this
class MYSQLPP_EXPORT Connection;
class MYSQLPP_EXPORT Row;
#endif

/// \brief Base class for all SSQLSv2 classes
///
/// Classes generated by ssqlsxlat derive from this class.  It is not
/// directly instantiable.  It exists only to hold the common interface
/// to all SSQLSv2 classes.
class MYSQLPP_EXPORT SsqlsBase
{
public:
	/// \brief Supported field subsets
	enum FieldSubset {
		fs_all,			///< all fields
		fs_key,			///< fields with "is key" attribute
		fs_set,			///< fields that have been given a value
		fs_not_autoinc	///< fields without "is autoinc" attribute
	};

	/// \brief Create table in database matching subclass schema
	///
	/// \param conn If given, use this connection instead of the one we
	/// may have gotten earlier; saves value for future use.
	///
	/// \return true if table was successfully created; on failure,
	/// only returns false if conn->throw_exceptions() is false.
	virtual bool create_table(Connection* conn = 0) const = 0;

	/// \brief Create record in database from all fields in object
	/// except for any marked as auto-increment.
	///
	/// \param conn If given, use this connection instead of the one we
	/// may have gotten earlier; saves value for future use.
	///
	/// \return true if new record was successfully created; on failure,
	/// only returns false if conn->throw_exceptions() is false.
	bool create(Connection* conn = 0) const;

	/// \brief Puts stream insertion operator into "equal list" mode
	///
	/// Typical usage:
	///
	/// \code cout << foo.equal_list() << endl;
	///
	/// \see operator<<(std::ostream&, const SsqlsBase&)
	const SsqlsBase& equal_list() const
	{
		output_mode_ = om_equal_list;
		return *this;
	}

	/// \brief Insert SSQLS's names and values into a stream.
	///
	/// An "equal list" is a set of name/value pairs in SQL assignment
	/// form, suitable for building INSERT queries and such.
	///
	/// \param os output stream to insert equal list into
	/// \param fs with default value, only the fields that have been
	/// given values are included in the output
	virtual std::ostream& equal_list(std::ostream& os,
			FieldSubset fs = fs_set) const = 0;

	/// \brief Set a per-instance SQL table name
	///
	/// This causes table() to return the passed value instead of the
	/// static per-class value it normally returns.  This is useful if
	/// you're using a single SSQLS definition for multiple tables that
	/// happen to share a common schema or schema subset.
	void instance_table(const char* name)
			{ instance_table_name_ = name; }

	/// \brief Retrieve record from the database matching our key field(s)
	///
	/// \param conn If given, use this connection instead of the one we
	/// may have gotten earlier; saves value for future use.
	///
	/// \return true if record was successfully retrieved; on failure,
	/// only returns false if conn->throw_exceptions() is false.
	bool load(Connection* conn = 0) const;

	/// \brief Puts stream insertion operator into "name list" mode
	///
	/// Typical usage:
	///
	/// \code cout << foo.name_list() << endl;
	///
	/// \see operator<<(std::ostream&, const SsqlsBase&)
	const SsqlsBase& name_list() const
	{
		output_mode_ = om_name_list;
		return *this;
	}

	/// \brief Insert the SSQLS's field names into a stream
	///
	/// A "name list" is a comma-separated list of SSQLS field names
	///
	/// \param os output stream to insert name list into
	/// \param fs with default value, only the names of fields that have
	/// been given values are included in the output
	virtual std::ostream& name_list(std::ostream& os,
			FieldSubset fs = fs_set) const = 0;

	/// \brief Returns truthy value if object's fields have been fully
	/// populated.
	///
	/// Test the Row, result or Query class you used to assign the
	/// value to distinguish a successful subset population from
	/// a complete failure to populate the object.  Because of this
	/// ambiguity, this operator is only useful for checking that an
	/// expected full population did in fact fully populate the object.
	/// This can help you to detect schema drift.
	operator const void*() const { return populated() ? this : 0; }

	/// \brief Returns true if the object has been populated
	///
	/// \param fs field subset to check, defaulting to "all fields"
	///
	/// This can give a different result than testing the object in
	/// bool context if you pass something other than fs_all.
	virtual bool populated(FieldSubset fs = fs_all) const = 0;

	/// \brief Delete record matching our key field(s) from the database
	///
	/// \param conn If given, use this connection instead of the one we
	/// may have gotten earlier; saves value for future use.
	///
	/// \return true if record was successfully removed; on failure,
	/// only returns false if conn->throw_exceptions() is false.
	bool remove(Connection* conn = 0) const;

	/// \brief Update record in database matching our key fields, or
	/// insert it if there is no such record.
	///
	/// \param conn If given, use this connection instead of the one we
	/// may have gotten earlier; saves value for future use.
	///
	/// \return true if record was successfully saved; on failure,
	/// only returns false if conn->throw_exceptions() is false.
	bool save(Connection* conn = 0) const;

	/// \brief Get the object's SQL table name
	///
	/// \r return the value set by instance_table(), if you called it,
	/// or a static per-class value otherwise
	const char* table() const { return instance_table_name_; }

	/// \brief Puts stream insertion operator into "value list" mode
	///
	/// Typical usage:
	///
	/// \code cout << foo.value_list() << endl;
	///
	/// \see operator<<(std::ostream&, const SsqlsBase&)
	const SsqlsBase& value_list() const
	{
		output_mode_ = om_value_list;
		return *this;
	}

	/// \brief Insert SSQLS's values into a stream.
	///
	/// A "value list" is a comma-separated list of SSQLS field values.
	/// Whether the values are quoted and/or escaped depends on the
	/// stream type passed for os.
	///
	/// \param os output stream to insert value list into
	/// \param fs with default value, only the fields that have been
	/// given values are included in the output
	virtual std::ostream& value_list(std::ostream& os,
			FieldSubset fs = fs_set) const = 0;

protected:
	/// \brief Default constructor
	///
	/// \param conn pointer to connection we should use for Active
	/// Record methods' queries, if no connection is passed for that
	/// particular call.
	SsqlsBase(Connection* conn = 0) :
	output_mode_(om_value_list),
	conn_(conn),
	instance_table_name_(0)
	{
	}

	/// \brief Full initialization constructor
	///
	/// \param row data to use in initializing SSQLS fields
	/// \param conn pointer to connection we should use for Active
	/// Record methods' queries, if no connection is passed for that
	/// particular call.
	/// \todo do something with row parameter
	SsqlsBase(const Row& row, Connection* conn = 0) :
	output_mode_(om_value_list),
	conn_(conn),
	instance_table_name_(0)
	{
		(void)row;
	}

	/// \brief Destructor
	virtual ~SsqlsBase() { }

	/// \brief Flag controlling the behavior of operator<<()
	mutable enum {
		om_equal_list,
		om_name_list,
		om_value_list
	} output_mode_;

	/// \brief Connection object we were initialized from, if any
	///
	/// This just provides a default for the Active Record methods
	/// above that take Connection*.
	mutable Connection* conn_;

private:
	/// \brief Table name override for this particular object
	const char* instance_table_name_;

	friend std::ostream& operator <<(std::ostream&, const SsqlsBase&);
};


/// \brief Put contents of an SSQLSv2 derivative into a \c std::ostream
///
/// If you use this operator directly, you get a comma-separated list of
/// field values in the ostream.  Whether those values are quoted and/or
/// escaped depends on the ostream type.
/// 
/// It is also useful to use this operator indirectly, via
/// SsqlsBase::equal_list() or SsqlsBase::name_list().  (There is also a
/// SsqlsBase::value_list(), but it's just an alias for calling this operator
/// directly.)  These methods set a flag on the object that causes this
/// operator to insert an equals list (e.g. "name1 = value1, name2 =
/// value2...") or a name list into the stream.  The flag is immediately
/// reset to give value lists again on completion of the insert operation.
/// For example:
///
/// \code
/// stock s = ...;	// initialize an SSQLSv2 object of 'stock' class
/// cout << "Set field names: " << s.name_list() << endl;
/// \endcode
///
/// The \c s.name_list() call sets the output mode to "names", then passes
/// \c cout and \c *this to this operator, which then calls the version
/// of s.name_list() taking a stream reference, which is overridden in
/// the leaf class to know how to insert the names of all fields set on
/// the \c s object.
///
/// \param os IOstream to insert object contents into
/// \param sb object to insert into the stream
std::ostream& operator <<(std::ostream& os, const SsqlsBase& sb)
{
	switch (sb.output_mode_) {
		case SsqlsBase::om_equal_list: sb.equal_list(os); break;
		case SsqlsBase::om_name_list:  sb.name_list(os);  break;
		case SsqlsBase::om_value_list: sb.value_list(os); break;
	}

	sb.output_mode_ = SsqlsBase::om_value_list;

	return os;
}

} // end namespace mysqlpp

#endif // !defined(MYSQLPP_SSQLS2_H)

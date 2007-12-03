/// \file result.h
/// \brief Declares classes for holding SQL query result sets.

/***********************************************************************
 Copyright (c) 1998 by Kevin Atkinson, (c) 1999-2001 by MySQL AB, and
 (c) 2004-2007 by Educational Technology Resources, Inc.  Others may
 also hold copyrights on code in this file.  See the CREDITS file in
 the top directory of the distribution for details.

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

#ifndef MYSQLPP_RESULT_H
#define MYSQLPP_RESULT_H

#include "common.h"

#include "exceptions.h"
#include "fields.h"
#include "field_names.h"
#include "field_types.h"
#include "noexceptions.h"
#include "refcounted.h"
#include "row.h"
#include "subiter.h"

namespace mysqlpp {

/// \brief Functor to call mysql_free_result() on the pointer you pass
/// to it.
///
/// This overrides RefCountedPointer's default destroyer, which uses
/// operator delete; it annoys the C API when you nuke its data
/// structures this way. :)
template <>
struct RefCountedPointerDestroyer<MYSQL_RES>
{
	/// \brief Functor implementation
	void operator()(MYSQL_RES* doomed)
	{
		if (doomed) {
			mysql_free_result(doomed);
		}
	}
};


/// \brief A basic result set class, for use with "use" queries.
///
/// A "use" query is one where you make the query and then process just
/// one row at a time in the result instead of dealing with them all as
/// a single large chunk.  (The name comes from the MySQL C API function
/// that initiates this action, \c mysql_use_result().)  By calling
/// fetch_row() until it throws a mysqlpp::BadQuery exception (or an
/// empty row if exceptions are disabled), you can process the result
/// set one row at a time.

class MYSQLPP_EXPORT ResUse : public OptionalExceptions
{
public:
	/// \brief Default constructor
	ResUse() :
	OptionalExceptions(),
	initialized_(false),
	fields_(this)
	{
	}
	
	/// \brief Create the object, fully initialized
	ResUse(MYSQL_RES* result, bool te = true);
	
	/// \brief Create a copy of another ResUse object
	ResUse(const ResUse& other) :
	OptionalExceptions(),
	initialized_(false)
	{
		copy(other);
	}
	
	/// \brief Destroy object
	virtual ~ResUse();

	/// \brief Copy another ResUse object's data into this object
	ResUse& operator =(const ResUse& other);

	/// \brief Returns the next row in a "use" query's result set
	///
	/// <b>Design weakness warning:</b> Although Result (returned from
	/// "store" queries) contains this method, it is of no use with such
	/// result sets.
	///
	/// This is a thick wrapper around mysql_fetch_row() in the MySQL
	/// C API.  It does a lot of error checking before returning the Row
	/// object containing the row data.  If you need the underlying C
	/// API row data, call fetch_raw_row() instead.
	Row fetch_row() const
	{
		if (!result_) {
			if (throw_exceptions()) {
				throw UseQueryError("Results not fetched");
			}
			else {
				return Row();
			}
		}
		MYSQL_ROW row = fetch_raw_row();
		const unsigned long* lengths = fetch_lengths();
		if (row && lengths) {
			return Row(row, this, lengths, throw_exceptions());
		}
		else {
			return Row();
		}
	}

	/// \brief Wraps mysql_fetch_row() in MySQL C API.
	///
	/// \internal You almost certainly want to call fetch_row() instead.
	/// It is anticipated that this is only useful within the library,
	/// to implement higher-level query types on top of raw "use"
	/// queries. Query::storein() uses it, for example.
	MYSQL_ROW fetch_raw_row() const
			{ return mysql_fetch_row(result_.raw()); }

	/// \brief Wraps mysql_fetch_lengths() in MySQL C API.
	const unsigned long* fetch_lengths() const
			{ return mysql_fetch_lengths(result_.raw()); }

	/// \brief Wraps mysql_fetch_field() in MySQL C API.
	const Field& fetch_field() const
			{ return *mysql_fetch_field(result_.raw()); }

	/// \brief Wraps mysql_field_seek() in MySQL C API.
	void field_seek(int field) const
			{ mysql_field_seek(result_.raw(), field); }

	/// \brief Wraps mysql_num_fields() in MySQL C API.
	int num_fields() const
			{ return mysql_num_fields(result_.raw()); }
	
	/// \brief Return the pointer to the underlying MySQL C API
	/// result set object.
	///
	/// While this has obvious inherent value for those times you need
	/// to dig beneath the MySQL++ interface, it has subtler value.
	/// It effectively stands in for operator bool(), operator !(),
	/// operator ==(), and operator !=(), because the C++ compiler can
	/// implement all of these with a MYSQL_RES*.
	///
	/// Of these uses, the most valuable is using the ResUse object in
	/// bool context to determine if the query that created it was
	/// successful:
	///
	/// \code
	///   Query q("....");
	///   if (ResUse res = q.use()) {
	///       // Can use 'res', query succeeded
	///   }
	///   else {
	///       // Query failed, call Query::error() or ::errnum() for why
	///   }
	/// \endcode
	operator MYSQL_RES*() const
	{
		return result_.raw();
	}
	
	/// \brief Return the name of the table the result set comes from
	const char* table() const
			{ return fields_.size() > 0 ? fields_[0].table : ""; }

	/// \brief Get the index of the named field.
	///
	/// This is the inverse of field_name().
	int field_num(const std::string&) const;

	/// \brief Get the name of the field at the given index.
	const std::string& field_name(int i) const
			{ return names_->at(i); }

	/// \brief Get the names of the fields within this result set.
	const RefCountedPointer<FieldNames>& field_names() const
			{ return names_; }

	/// \brief Get the MySQL type for a field given its index.
	const mysql_type_info& field_type(int i) const
			{ return types_->at(i); }

	/// \brief Get a list of the types of the fields within this
	/// result set.
	const RefCountedPointer<FieldTypes>& field_types() const
			{ return types_; }

	/// \brief Get the underlying Fields structure.
	const Fields& fields() const { return fields_; }

	/// \brief Get the underlying Field structure given its index.
	const Field& field(unsigned int i) const { return fields_.at(i); }

protected:
	bool initialized_;			///< if true, object is fully initted
	Fields fields_;				///< list of fields in result

	/// \brief underlying C API result set
	///
	/// This is mutable because so many methods in this class and in
	/// Result are justifiably const, but they call C API methods that
	/// take non-const MYSQL_RES*.  It's possible (likely even, in many
	/// cases) that these functions do modify the MYSQL_RES object
	/// which is part of this object, so strict constness says this
	/// object changed, too, but this has always been mutable and the
	/// resulting behavior hasn't confused anyone yet.  I think this is
	/// because people the methods likely to change the C API object are
	/// those used in "use" queries, but MySQL++ users tend to treat
	/// ResUse objects the same as Result objects: it's a convenient
	/// fiction that the entire result set is present in both cases, so
	/// the act of fetching new rows is an implementation detail that
	/// doesn't modify the function of the object.  Thus, it is
	/// effectively still const.
	mutable RefCountedPointer<MYSQL_RES> result_;

	/// \brief list of field names in result
	RefCountedPointer<FieldNames> names_;

	/// \brief list of field types in result
	RefCountedPointer<FieldTypes> types_;

	/// \brief Copy another ResUse object's contents into this one.
	///
	/// Self-copy is not allowed.
	void copy(const ResUse& other);
};


/// \brief This class manages SQL result sets. 
///
/// Objects of this class are created to manage the result of "store"
/// queries, where the result set is handed to the program as single
/// block of row data. (The name comes from the MySQL C API function
/// \c mysql_store_result() which creates these blocks of row data.)
///
/// This class is a random access container (in the STL sense) which
/// is neither less-than comparable nor assignable.  This container
/// provides a reverse random-access iterator in addition to the normal
/// forward one.

class MYSQLPP_EXPORT Result : public ResUse
{
public:
	typedef int difference_type;			///< type for index differences
	typedef unsigned int size_type;			///< type of returned sizes

	typedef Row value_type;					///< type of data in container
	typedef value_type& reference;			///< reference to value_type
	typedef const value_type& const_reference;///< const ref to value_type
	typedef value_type* pointer;			///< pointer to value_type
	typedef const value_type* const_pointer;///< const pointer to value_type

	/// \brief regular iterator type
	///
	/// Note that this is the same as const_iterator; we don't have a
	/// mutable iterator type.
	typedef subscript_iterator<const Result, const value_type, size_type,
			difference_type> iterator;	
	typedef iterator const_iterator;		///< constant iterator type

	/// \brief mutable reverse iterator type
	typedef const std::reverse_iterator<iterator> reverse_iterator;			

	/// \brief const reverse iterator type
	typedef const std::reverse_iterator<const_iterator> const_reverse_iterator;		

	/// \brief Return maximum number of elements that can be stored
	/// in container without resizing.
	size_type max_size() const { return size(); }

	/// \brief Returns true if container is empty
	bool empty() const { return size() == 0; }

	/// \brief Return iterator pointing to first element in the
	/// container
	iterator begin() const { return iterator(this, 0); }

	/// \brief Return iterator pointing to one past the last element
	/// in the container
	iterator end() const { return iterator(this, size()); }

	/// \brief Return reverse iterator pointing to first element in the
	/// container
	reverse_iterator rbegin() const { return reverse_iterator(end()); }

	/// \brief Return reverse iterator pointing to one past the last
	/// element in the container
	reverse_iterator rend() const { return reverse_iterator(begin()); }

	/// \brief Default constructor
	Result()
	{
	}
	
	/// \brief Fully initialize object
	Result(MYSQL_RES* result, bool te = true) :
	ResUse(result, te)
	{
	}

	/// \brief Initialize object as a copy of another Result object
	Result(const Result& other) :
	ResUse(other)
	{
	}

	/// \brief Destroy result set
	virtual ~Result() { }

	/// \brief Wraps mysql_num_rows() in MySQL C API.
	my_ulonglong num_rows() const
	{
		return initialized_ ? mysql_num_rows(result_.raw()) : 0;
	}

	/// \brief Wraps mysql_data_seek() in MySQL C API.
	void data_seek(uint offset) const
	{
		mysql_data_seek(result_.raw(), offset);
	}

	/// \brief Alias for num_rows(), only with different return type.
	size_type size() const { return static_cast<size_type>(num_rows()); }

	/// \brief Get the row with an offset of i.
	const value_type at(int i) const
	{
		data_seek(i);
		return fetch_row();
	}

	/// \brief Get the row with an offset of i.
	///
	/// Just a synonym for at()
	const value_type operator [](int i) const { return at(i); }
};


/// \brief Swaps two ResUse objects
inline void swap(ResUse& x, ResUse& y)
{
	ResUse tmp = x;
	x = y;
	y = tmp;
}

/// \brief Swaps two Result objects
inline void swap(Result& x, Result& y)
{
	Result tmp = x;
	x = y;
	y = tmp;
}

/// \brief Holds information on queries that don't return data.
class MYSQLPP_EXPORT ResNSel
{
private:
	/// \brief Pointer to bool data member, for use by safe bool
	/// conversion operator.
	///
	/// \see http://www.artima.com/cppsource/safebool.html
    typedef bool ResNSel::*private_bool_type;

public:
	/// \brief Default ctor
	ResNSel() :
	copacetic_(false),
	insert_id_(0),
	rows_(0)
	{
	}

	/// \brief Initialize object
	ResNSel(bool copacetic, my_ulonglong insert_id,
			my_ulonglong rows, const std::string& info) :
	copacetic_(copacetic),
	insert_id_(insert_id),
	rows_(rows),
	info_(info)
	{
	}

	/// \brief Test whether the query that created this result succeeded
	///
	/// If you test this object in bool context and it's false, it's a
	/// signal that the query this was created from failed in some way.
	/// Call Query::error() or Query::errnum() to find out what exactly
	/// happened.
	operator private_bool_type() const
	{
		return copacetic_ ? &ResNSel::copacetic_ : 0;
	}

	/// \brief Get the last value used for an AUTO_INCREMENT field
	my_ulonglong insert_id() const { return insert_id_; }

	/// \brief Get the number of rows affected by the query
	my_ulonglong rows() const { return rows_; }

	/// \brief Get any additional information about the query returned
	/// by the server.
	const char* info() const { return info_.c_str(); }

private:
	bool copacetic_;
	my_ulonglong insert_id_;
	my_ulonglong rows_;
	std::string info_;
};


} // end namespace mysqlpp

#endif

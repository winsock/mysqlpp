/// \file mysql++.h
/// \brief The main MySQL++ header file.  It simply #includes all of the
/// other header files (except for custom.h) in the proper order.
///
/// Most programs will have no reason to #include any of the other
/// MySQL++ headers directly, except for custom.h.

#include "platform.h"

#include "defs.h"

#include "coldata.h"
#include "compare.h"
#include "connection.h"
#include "const_string.h"
#include "convert.h"
#include "datetime.h"
#include "exceptions.h"
#include "field_names.h"
#include "field_types.h"
#include "fields.h"
#include "manip.h"
#include "myset.h"
#include "null.h"
#include "query.h"
#include "resiter.h"
#include "result.h"
#include "row.h"
#include "sql_query.h"
#include "sql_string.h"
#include "stream2string.h"
#include "tiny_int.h"
#include "type_info.h"
#include "vallist.h"


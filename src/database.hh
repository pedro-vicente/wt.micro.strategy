#ifndef DB_INTERFACE_HH
#define DB_INTERFACE_HH

#include <string>
#include <vector>
#include <map>
#include "finmart.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
// DatabaseBackend
// database backend types
/////////////////////////////////////////////////////////////////////////////////////////////////////

enum class DatabaseBackend
{
  SQLITE,
  SQLSERVER
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
// IFinMartDatabase
// supports both SQLite and SQL Server backends
/////////////////////////////////////////////////////////////////////////////////////////////////////

class IFinMartDatabase
{
public:
  virtual ~IFinMartDatabase() = default;

  virtual bool is_open() const = 0;
  virtual std::vector<transaction> get_all_transactions() = 0;
  virtual std::map<std::string, double> get_department_spending() = 0;
  virtual std::map<std::string, int> get_source_system_counts() = 0;
  virtual double get_total_spending() = 0;

  virtual DatabaseBackend get_backend() const = 0;
  virtual std::string get_backend_name() const = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
// DatabaseFactory
/////////////////////////////////////////////////////////////////////////////////////////////////////

class DatabaseFactory
{
public:
  // Create database based on backend type
  // For SQLite: connection_string is the filename (e.g., "finmart.db")
  // For SQL Server: connection_string is ODBC connection string
  static IFinMartDatabase* create(DatabaseBackend backend, const std::string& connection_string);
};

#endif

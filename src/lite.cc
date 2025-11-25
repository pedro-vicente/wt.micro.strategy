#include "lite.hh"

/////////////////////////////////////////////////////////////////////////////////////////////////////
// finmart_db
/////////////////////////////////////////////////////////////////////////////////////////////////////

finmart_db::finmart_db(const std::string& db_path)
{
  int rc = sqlite3_open(db_path.c_str(), &db);
  if (rc)
  {
    db = nullptr;
  }
  else
  {
    initialize_database();
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// ~finmart_db
/////////////////////////////////////////////////////////////////////////////////////////////////////

finmart_db::~finmart_db()
{
  if (db)
  {
    sqlite3_close(db);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// initialize_database
/////////////////////////////////////////////////////////////////////////////////////////////////////

void finmart_db::initialize_database()
{
  const char* create_table = R"(
      CREATE TABLE IF NOT EXISTS transactions (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          date TEXT NOT NULL,
          department TEXT NOT NULL,
          category TEXT NOT NULL,
          vendor TEXT NOT NULL,
          amount REAL NOT NULL,
          status TEXT NOT NULL,
          source_system TEXT NOT NULL
      );
  )";

  char* err_msg = nullptr;
  int rc = sqlite3_exec(db, create_table, nullptr, nullptr, &err_msg);
  if (rc != SQLITE_OK)
  {
    sqlite3_free(err_msg);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // check if we need to populate sample data
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  const char* count_query = "SELECT COUNT(*) FROM transactions;";
  sqlite3_stmt* stmt;
  sqlite3_prepare_v2(db, count_query, -1, &stmt, nullptr);
  if (sqlite3_step(stmt) == SQLITE_ROW)
  {
    int count = sqlite3_column_int(stmt, 0);
    if (count == 0)
    {
      populate_sample_data();
    }
  }
  sqlite3_finalize(stmt);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// populate_sample_data
/////////////////////////////////////////////////////////////////////////////////////////////////////

void finmart_db::populate_sample_data()
{
  std::vector<std::string> departments = { "IT", "HR", "Finance", "Operations", "Procurement" };
  std::vector<std::string> categories = { "Software", "Hardware", "Services", "Travel", "Supplies" };
  std::vector<std::string> vendors = { "Microsoft", "Dell", "AWS", "Oracle", "Cisco", "Adobe" };
  std::vector<std::string> systems = { "PeopleSoft", "Coupa", "SAP", "Legacy" };

  const char* sql = "INSERT INTO transactions (date, department, category, vendor, amount, status, source_system) VALUES (?, ?, ?, ?, ?, ?, ?);";

  for (int idx = 0; idx < 50; idx++)
  {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    std::stringstream date_str;
    date_str << "2025-" << std::setfill('0') << std::setw(2) << ((idx % 12) + 1)
      << "-" << std::setfill('0') << std::setw(2) << ((idx % 28) + 1);

    sqlite3_bind_text(stmt, 1, date_str.str().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, departments[idx % departments.size()].c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, categories[idx % categories.size()].c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, vendors[idx % vendors.size()].c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 5, (rand() % 50000 + 1000) / 100.0);
    sqlite3_bind_text(stmt, 6, (idx % 10 == 0) ? "Pending" : "Approved", -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, systems[idx % systems.size()].c_str(), -1, SQLITE_TRANSIENT);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// get_all_transactions
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<transaction> finmart_db::get_all_transactions()
{
  std::vector<transaction> transactions;
  if (!db) return transactions;

  const char* query = "SELECT * FROM transactions ORDER BY date DESC LIMIT 100;";

  sqlite3_stmt* stmt;
  sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);

  while (sqlite3_step(stmt) == SQLITE_ROW)
  {
    transaction t;
    t.id = sqlite3_column_int(stmt, 0);
    t.date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    t.department = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    t.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    t.vendor = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    t.amount = sqlite3_column_double(stmt, 5);
    t.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
    t.source_system = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
    transactions.push_back(t);
  }

  sqlite3_finalize(stmt);
  return transactions;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// get_department_spending
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::map<std::string, double> finmart_db::get_department_spending()
{
  std::map<std::string, double> spending;
  if (!db) return spending;

  const char* query = "SELECT department, SUM(amount) FROM transactions GROUP BY department;";

  sqlite3_stmt* stmt;
  sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);

  while (sqlite3_step(stmt) == SQLITE_ROW)
  {
    std::string dept = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    double total = sqlite3_column_double(stmt, 1);
    spending[dept] = total;
  }

  sqlite3_finalize(stmt);
  return spending;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// get_source_system_counts
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::map<std::string, int> finmart_db::get_source_system_counts()
{
  std::map<std::string, int> counts;
  if (!db) return counts;

  const char* query = "SELECT source_system, COUNT(*) FROM transactions GROUP BY source_system;";

  sqlite3_stmt* stmt;
  sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);

  while (sqlite3_step(stmt) == SQLITE_ROW)
  {
    std::string system = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    int count = sqlite3_column_int(stmt, 1);
    counts[system] = count;
  }

  sqlite3_finalize(stmt);
  return counts;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// get_total_spending
/////////////////////////////////////////////////////////////////////////////////////////////////////

double finmart_db::get_total_spending()
{
  if (!db) return 0.0;

  const char* query = "SELECT SUM(amount) FROM transactions;";
  sqlite3_stmt* stmt;
  sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);

  double total = 0.0;
  if (sqlite3_step(stmt) == SQLITE_ROW)
  {
    total = sqlite3_column_double(stmt, 0);
  }

  sqlite3_finalize(stmt);
  return total;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// is_open - check if database opened successfully
/////////////////////////////////////////////////////////////////////////////////////////////////////

bool finmart_db::is_open() const
{
  return db != nullptr;
}


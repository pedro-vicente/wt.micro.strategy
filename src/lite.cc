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
  // create financial_records table
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  const char* create_financial_table = R"(
      CREATE TABLE IF NOT EXISTS financial_records (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          period TEXT NOT NULL,
          company_id TEXT NOT NULL,
          revenue REAL NOT NULL,
          cogs REAL NOT NULL,
          operating_expenses REAL NOT NULL,
          depreciation REAL NOT NULL,
          amortization REAL NOT NULL,
          interest REAL NOT NULL,
          taxes REAL NOT NULL,
          current_assets REAL NOT NULL,
          current_liabilities REAL NOT NULL,
          inventory REAL NOT NULL,
          total_assets REAL NOT NULL,
          total_liabilities REAL NOT NULL
      );
  )";

  rc = sqlite3_exec(db, create_financial_table, nullptr, nullptr, &err_msg);
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

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // check if we need to populate financial data
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  const char* fin_count_query = "SELECT COUNT(*) FROM financial_records;";
  sqlite3_prepare_v2(db, fin_count_query, -1, &stmt, nullptr);
  if (sqlite3_step(stmt) == SQLITE_ROW)
  {
    int count = sqlite3_column_int(stmt, 0);
    if (count == 0)
    {
      populate_financial_data();
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

/////////////////////////////////////////////////////////////////////////////////////////////////////
// populate_financial_data - sample financial records for multiple companies and periods
/////////////////////////////////////////////////////////////////////////////////////////////////////

void finmart_db::populate_financial_data()
{
  std::vector<std::string> companies = { "ACME Corp", "TechVentures", "GlobalTrade" };
  std::vector<std::string> periods = { "2024-Q1", "2024-Q2", "2024-Q3", "2024-Q4", "2025-Q1" };

  const char* sql = R"(
    INSERT INTO financial_records 
    (period, company_id, revenue, cogs, operating_expenses, depreciation, amortization,
     interest, taxes, current_assets, current_liabilities, inventory, total_assets, total_liabilities)
    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
  )";

  double base_revenue = 1000000.0;

  for (size_t cidx = 0; cidx < companies.size(); ++cidx)
  {
    double company_factor = 1.0 + (cidx * 0.5);

    for (size_t pidx = 0; pidx < periods.size(); ++pidx)
    {
      double growth = 1.0 + (pidx * 0.05);

      sqlite3_stmt* stmt;
      sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

      double revenue = base_revenue * company_factor * growth;
      double cogs = revenue * (0.55 + (rand() % 10) / 100.0);
      double opex = revenue * (0.20 + (rand() % 5) / 100.0);
      double depr = revenue * 0.03;
      double amort = revenue * 0.02;
      double interest_exp = revenue * 0.02;
      double taxes_exp = (revenue - cogs - opex - depr - amort - interest_exp) * 0.25;
      double curr_assets = revenue * 0.4;
      double curr_liab = revenue * 0.25;
      double inv = revenue * 0.15;
      double tot_assets = revenue * 2.0;
      double tot_liab = revenue * 0.8;

      sqlite3_bind_text(stmt, 1, periods[pidx].c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 2, companies[cidx].c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_double(stmt, 3, revenue);
      sqlite3_bind_double(stmt, 4, cogs);
      sqlite3_bind_double(stmt, 5, opex);
      sqlite3_bind_double(stmt, 6, depr);
      sqlite3_bind_double(stmt, 7, amort);
      sqlite3_bind_double(stmt, 8, interest_exp);
      sqlite3_bind_double(stmt, 9, taxes_exp);
      sqlite3_bind_double(stmt, 10, curr_assets);
      sqlite3_bind_double(stmt, 11, curr_liab);
      sqlite3_bind_double(stmt, 12, inv);
      sqlite3_bind_double(stmt, 13, tot_assets);
      sqlite3_bind_double(stmt, 14, tot_liab);

      sqlite3_step(stmt);
      sqlite3_finalize(stmt);
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// get_financial_records - retrieve all financial records
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<FinancialRecord> finmart_db::get_financial_records()
{
  std::vector<FinancialRecord> records;
  if (!db) return records;

  const char* query = "SELECT period, company_id, revenue, cogs, operating_expenses, "
    "depreciation, amortization, interest, taxes, current_assets, current_liabilities, "
    "inventory, total_assets, total_liabilities FROM financial_records ORDER BY company_id, period;";

  sqlite3_stmt* stmt;
  sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);

  while (sqlite3_step(stmt) == SQLITE_ROW)
  {
    FinancialRecord r;
    r.period = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    r.company_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    r.revenue = sqlite3_column_double(stmt, 2);
    r.cogs = sqlite3_column_double(stmt, 3);
    r.operating_expenses = sqlite3_column_double(stmt, 4);
    r.depreciation = sqlite3_column_double(stmt, 5);
    r.amortization = sqlite3_column_double(stmt, 6);
    r.interest = sqlite3_column_double(stmt, 7);
    r.taxes = sqlite3_column_double(stmt, 8);
    r.current_assets = sqlite3_column_double(stmt, 9);
    r.current_liabilities = sqlite3_column_double(stmt, 10);
    r.inventory = sqlite3_column_double(stmt, 11);
    r.total_assets = sqlite3_column_double(stmt, 12);
    r.total_liabilities = sqlite3_column_double(stmt, 13);
    records.push_back(r);
  }

  sqlite3_finalize(stmt);
  return records;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// get_financial_records_by_company
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<FinancialRecord> finmart_db::get_financial_records_by_company(const std::string& company_id)
{
  std::vector<FinancialRecord> records;
  if (!db) return records;

  const char* query = "SELECT period, company_id, revenue, cogs, operating_expenses, "
    "depreciation, amortization, interest, taxes, current_assets, current_liabilities, "
    "inventory, total_assets, total_liabilities FROM financial_records WHERE company_id = ? ORDER BY period;";

  sqlite3_stmt* stmt;
  sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
  sqlite3_bind_text(stmt, 1, company_id.c_str(), -1, SQLITE_TRANSIENT);

  while (sqlite3_step(stmt) == SQLITE_ROW)
  {
    FinancialRecord r;
    r.period = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    r.company_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    r.revenue = sqlite3_column_double(stmt, 2);
    r.cogs = sqlite3_column_double(stmt, 3);
    r.operating_expenses = sqlite3_column_double(stmt, 4);
    r.depreciation = sqlite3_column_double(stmt, 5);
    r.amortization = sqlite3_column_double(stmt, 6);
    r.interest = sqlite3_column_double(stmt, 7);
    r.taxes = sqlite3_column_double(stmt, 8);
    r.current_assets = sqlite3_column_double(stmt, 9);
    r.current_liabilities = sqlite3_column_double(stmt, 10);
    r.inventory = sqlite3_column_double(stmt, 11);
    r.total_assets = sqlite3_column_double(stmt, 12);
    r.total_liabilities = sqlite3_column_double(stmt, 13);
    records.push_back(r);
  }

  sqlite3_finalize(stmt);
  return records;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// insert_financial_record
/////////////////////////////////////////////////////////////////////////////////////////////////////

int finmart_db::insert_financial_record(const FinancialRecord& record)
{
  if (!db) return -1;

  const char* sql = R"(
    INSERT INTO financial_records 
    (period, company_id, revenue, cogs, operating_expenses, depreciation, amortization,
     interest, taxes, current_assets, current_liabilities, inventory, total_assets, total_liabilities)
    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
  )";

  sqlite3_stmt* stmt;
  sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

  sqlite3_bind_text(stmt, 1, record.period.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, record.company_id.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_double(stmt, 3, record.revenue);
  sqlite3_bind_double(stmt, 4, record.cogs);
  sqlite3_bind_double(stmt, 5, record.operating_expenses);
  sqlite3_bind_double(stmt, 6, record.depreciation);
  sqlite3_bind_double(stmt, 7, record.amortization);
  sqlite3_bind_double(stmt, 8, record.interest);
  sqlite3_bind_double(stmt, 9, record.taxes);
  sqlite3_bind_double(stmt, 10, record.current_assets);
  sqlite3_bind_double(stmt, 11, record.current_liabilities);
  sqlite3_bind_double(stmt, 12, record.inventory);
  sqlite3_bind_double(stmt, 13, record.total_assets);
  sqlite3_bind_double(stmt, 14, record.total_liabilities);

  int rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);

  return (rc == SQLITE_DONE) ? 0 : -1;
}


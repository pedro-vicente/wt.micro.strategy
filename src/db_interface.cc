#include "db_interface.hh"
#include "lite.hh"
#include "odbc.hh"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <random>

/////////////////////////////////////////////////////////////////////////////////////////////////////
// FinMartSQLite - SQLite implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////

class FinMartSQLite : public IFinMartDatabase
{
private:
  finmart_db* db;

public:
  FinMartSQLite(const std::string& filename)
  {
    db = new finmart_db(filename);
  }

  ~FinMartSQLite()
  {
    delete db;
  }

  bool is_open() const override
  {
    return db->is_open();
  }

  std::vector<transaction> get_all_transactions() override
  {
    return db->get_all_transactions();
  }

  std::map<std::string, double> get_department_spending() override
  {
    return db->get_department_spending();
  }

  std::map<std::string, int> get_source_system_counts() override
  {
    return db->get_source_system_counts();
  }

  double get_total_spending() override
  {
    return db->get_total_spending();
  }

  DatabaseBackend get_backend() const override
  {
    return DatabaseBackend::SQLITE;
  }

  std::string get_backend_name() const override
  {
    return "SQLite";
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // financial record methods
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::vector<FinancialRecord> get_financial_records() override
  {
    return db->get_financial_records();
  }

  std::vector<FinancialRecord> get_financial_records_by_company(const std::string& company_id) override
  {
    return db->get_financial_records_by_company(company_id);
  }

  int insert_financial_record(const FinancialRecord& record) override
  {
    return db->insert_financial_record(record);
  }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
// FinMartSQLServer - SQL Server ODBC implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////

class FinMartSQLServer : public IFinMartDatabase
{
private:
  odbc db;
  bool connected;

  void initialize_schema()
  {

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // create transactions table if it doesn't exist
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    std::string sql = R"(
            IF NOT EXISTS (SELECT * FROM sys.tables WHERE name = 'transactions')
            BEGIN
                CREATE TABLE transactions (
                    id INT PRIMARY KEY IDENTITY(1,1),
                    date VARCHAR(20) NOT NULL,
                    department VARCHAR(50) NOT NULL,
                    category VARCHAR(50) NOT NULL,
                    vendor VARCHAR(100) NOT NULL,
                    amount FLOAT NOT NULL,
                    status VARCHAR(20) NOT NULL,
                    source_system VARCHAR(50) NOT NULL
                )
            END
        )";
    db.exec_direct(sql);

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // create financial_records table if it doesn't exist
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    std::string fin_sql = R"(
            IF NOT EXISTS (SELECT * FROM sys.tables WHERE name = 'financial_records')
            BEGIN
                CREATE TABLE financial_records (
                    id INT PRIMARY KEY IDENTITY(1,1),
                    period VARCHAR(20) NOT NULL,
                    company_id VARCHAR(100) NOT NULL,
                    revenue FLOAT NOT NULL,
                    cogs FLOAT NOT NULL,
                    operating_expenses FLOAT NOT NULL,
                    depreciation FLOAT NOT NULL,
                    amortization FLOAT NOT NULL,
                    interest FLOAT NOT NULL,
                    taxes FLOAT NOT NULL,
                    current_assets FLOAT NOT NULL,
                    current_liabilities FLOAT NOT NULL,
                    inventory FLOAT NOT NULL,
                    total_assets FLOAT NOT NULL,
                    total_liabilities FLOAT NOT NULL
                )
            END
        )";
    db.exec_direct(fin_sql);

    table_t check;
    db.fetch("SELECT COUNT(*) as cnt FROM transactions", check);
    if (check.rows.size() > 0 && check.rows[0].col[0] == "0")
    {
      populate_sample_data();
    }

    table_t fin_check;
    db.fetch("SELECT COUNT(*) as cnt FROM financial_records", fin_check);
    if (fin_check.rows.size() > 0 && fin_check.rows[0].col[0] == "0")
    {
      populate_financial_data();
    }
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // populate_sample_data
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  void populate_sample_data() 
  {
    const char* departments[] = { "IT", "HR", "Finance", "Operations", "Procurement" };
    const char* categories[] = { "Software", "Hardware", "Services", "Travel", "Supplies", "Training" };
    const char* vendors[] = { "Microsoft", "Dell", "AWS", "Oracle", "Cisco", "Adobe", "SAP", "IBM" };
    const char* sources[] = { "PeopleSoft", "Coupa", "SAP", "MicroStrategy" };
    const char* statuses[] = { "Approved", "Pending" };

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dept_dist(0, 4);
    std::uniform_int_distribution<> cat_dist(0, 5);
    std::uniform_int_distribution<> vendor_dist(0, 7);
    std::uniform_int_distribution<> source_dist(0, 3);
    std::uniform_int_distribution<> status_dist(0, 1);
    std::uniform_real_distribution<> amount_dist(100.0, 50000.0);

    for (int idx = 0; idx < 50; idx++) {
      std::time_t t = std::time(nullptr) - (idx * 86400);
      std::tm* tm = std::localtime(&t);
      char date_buf[20];
      std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", tm);

      std::stringstream sql;
      sql << "INSERT INTO transactions (date, department, category, vendor, amount, status, source_system) VALUES ('"
        << date_buf << "', '"
        << departments[dept_dist(gen)] << "', '"
        << categories[cat_dist(gen)] << "', '"
        << vendors[vendor_dist(gen)] << "', "
        << amount_dist(gen) << ", '"
        << statuses[status_dist(gen)] << "', '"
        << sources[source_dist(gen)] << "')";

      db.exec_direct(sql.str());
    }
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // populate_financial_data
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  void populate_financial_data()
  {
    const char* companies[] = { "ACME Corp", "TechVentures", "GlobalTrade" };
    const char* periods[] = { "2024-Q1", "2024-Q2", "2024-Q3", "2024-Q4", "2025-Q1" };
    double base_revenue = 1000000.0;

    std::random_device rd;
    std::mt19937 gen(rd());

    for (int cidx = 0; cidx < 3; ++cidx)
    {
      double company_factor = 1.0 + (cidx * 0.5);

      for (int pidx = 0; pidx < 5; ++pidx)
      {
        double growth = 1.0 + (pidx * 0.05);
        double revenue = base_revenue * company_factor * growth;
        double cogs = revenue * (0.55 + (gen() % 10) / 100.0);
        double opex = revenue * (0.20 + (gen() % 5) / 100.0);
        double depr = revenue * 0.03;
        double amort = revenue * 0.02;
        double interest_exp = revenue * 0.02;
        double taxes_exp = (revenue - cogs - opex - depr - amort - interest_exp) * 0.25;
        double curr_assets = revenue * 0.4;
        double curr_liab = revenue * 0.25;
        double inv = revenue * 0.15;
        double tot_assets = revenue * 2.0;
        double tot_liab = revenue * 0.8;

        std::stringstream sql;
        sql << std::fixed << std::setprecision(2);
        sql << "INSERT INTO financial_records (period, company_id, revenue, cogs, operating_expenses, "
          << "depreciation, amortization, interest, taxes, current_assets, current_liabilities, "
          << "inventory, total_assets, total_liabilities) VALUES ('"
          << periods[pidx] << "', '" << companies[cidx] << "', "
          << revenue << ", " << cogs << ", " << opex << ", " << depr << ", " << amort << ", "
          << interest_exp << ", " << taxes_exp << ", " << curr_assets << ", " << curr_liab << ", "
          << inv << ", " << tot_assets << ", " << tot_liab << ")";

        db.exec_direct(sql.str());
      }
    }
  }

public:
  FinMartSQLServer(const std::string& connection_string) : connected(false)
  {
    if (db.connect(connection_string) == 0) 
    {
      connected = true;
      initialize_schema();
    }
  }

  ~FinMartSQLServer()
  {
    if (connected)
    {
      db.disconnect();
    }
  }

  bool is_open() const override
  {
    return connected;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // get_all_transactions
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::vector<transaction> get_all_transactions() override
  {
    std::vector<transaction> transactions;
    table_t table;

    if (db.fetch("SELECT id, date, department, category, vendor, amount, status, source_system FROM transactions ORDER BY date DESC", table) == 0) {
      for (size_t idx = 0; idx < table.rows.size(); idx++) {
        const row_t& row = table.rows[idx];
        if (row.col.size() >= 8) {
          transaction t;
          t.id = std::stoi(row.col[0]);
          t.date = row.col[1];
          t.department = row.col[2];
          t.category = row.col[3];
          t.vendor = row.col[4];
          t.amount = std::stod(row.col[5]);
          t.status = row.col[6];
          t.source_system = row.col[7];
          transactions.push_back(t);
        }
      }
    }

    return transactions;
  }

  std::map<std::string, double> get_department_spending() override
  {
    std::map<std::string, double> spending;
    table_t table;

    if (db.fetch("SELECT department, SUM(amount) as total FROM transactions GROUP BY department ORDER BY total DESC", table) == 0) {
      for (size_t idx = 0; idx < table.rows.size(); idx++) {
        const row_t& row = table.rows[idx];
        if (row.col.size() >= 2) {
          spending[row.col[0]] = std::stod(row.col[1]);
        }
      }
    }

    return spending;
  }

  std::map<std::string, int> get_source_system_counts() override
  {
    std::map<std::string, int> counts;
    table_t table;

    if (db.fetch("SELECT source_system, COUNT(*) as cnt FROM transactions GROUP BY source_system", table) == 0) {
      for (size_t idx = 0; idx < table.rows.size(); idx++) {
        const row_t& row = table.rows[idx];
        if (row.col.size() >= 2) {
          counts[row.col[0]] = std::stoi(row.col[1]);
        }
      }
    }

    return counts;
  }

  double get_total_spending() override
  {
    table_t table;

    if (db.fetch("SELECT SUM(amount) as total FROM transactions", table) == 0)
    {
      if (table.rows.size() > 0 && table.rows[0].col.size() > 0) 
      {
        return std::stod(table.rows[0].col[0]);
      }
    }

    return 0.0;
  }

  DatabaseBackend get_backend() const override 
  {
    return DatabaseBackend::SQLSERVER;
  }

  std::string get_backend_name() const override
  {
    return "SQL Server";
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // financial record methods
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::vector<FinancialRecord> get_financial_records() override
  {
    std::vector<FinancialRecord> records;
    table_t table;

    std::string query = "SELECT period, company_id, revenue, cogs, operating_expenses, "
      "depreciation, amortization, interest, taxes, current_assets, current_liabilities, "
      "inventory, total_assets, total_liabilities FROM financial_records ORDER BY company_id, period";

    if (db.fetch(query, table) == 0)
    {
      for (size_t idx = 0; idx < table.rows.size(); ++idx)
      {
        const row_t& row = table.rows[idx];
        if (row.col.size() >= 14)
        {
          FinancialRecord r;
          r.period = row.col[0];
          r.company_id = row.col[1];
          r.revenue = std::stod(row.col[2]);
          r.cogs = std::stod(row.col[3]);
          r.operating_expenses = std::stod(row.col[4]);
          r.depreciation = std::stod(row.col[5]);
          r.amortization = std::stod(row.col[6]);
          r.interest = std::stod(row.col[7]);
          r.taxes = std::stod(row.col[8]);
          r.current_assets = std::stod(row.col[9]);
          r.current_liabilities = std::stod(row.col[10]);
          r.inventory = std::stod(row.col[11]);
          r.total_assets = std::stod(row.col[12]);
          r.total_liabilities = std::stod(row.col[13]);
          records.push_back(r);
        }
      }
    }

    return records;
  }

  std::vector<FinancialRecord> get_financial_records_by_company(const std::string& company_id) override
  {
    std::vector<FinancialRecord> records;
    table_t table;

    std::string query = "SELECT period, company_id, revenue, cogs, operating_expenses, "
      "depreciation, amortization, interest, taxes, current_assets, current_liabilities, "
      "inventory, total_assets, total_liabilities FROM financial_records WHERE company_id = '"
      + company_id + "' ORDER BY period";

    if (db.fetch(query, table) == 0)
    {
      for (size_t idx = 0; idx < table.rows.size(); ++idx)
      {
        const row_t& row = table.rows[idx];
        if (row.col.size() >= 14)
        {
          FinancialRecord r;
          r.period = row.col[0];
          r.company_id = row.col[1];
          r.revenue = std::stod(row.col[2]);
          r.cogs = std::stod(row.col[3]);
          r.operating_expenses = std::stod(row.col[4]);
          r.depreciation = std::stod(row.col[5]);
          r.amortization = std::stod(row.col[6]);
          r.interest = std::stod(row.col[7]);
          r.taxes = std::stod(row.col[8]);
          r.current_assets = std::stod(row.col[9]);
          r.current_liabilities = std::stod(row.col[10]);
          r.inventory = std::stod(row.col[11]);
          r.total_assets = std::stod(row.col[12]);
          r.total_liabilities = std::stod(row.col[13]);
          records.push_back(r);
        }
      }
    }

    return records;
  }

  int insert_financial_record(const FinancialRecord& record) override
  {
    std::stringstream sql;
    sql << std::fixed << std::setprecision(2);
    sql << "INSERT INTO financial_records (period, company_id, revenue, cogs, operating_expenses, "
      << "depreciation, amortization, interest, taxes, current_assets, current_liabilities, "
      << "inventory, total_assets, total_liabilities) VALUES ('"
      << record.period << "', '" << record.company_id << "', "
      << record.revenue << ", " << record.cogs << ", " << record.operating_expenses << ", "
      << record.depreciation << ", " << record.amortization << ", " << record.interest << ", "
      << record.taxes << ", " << record.current_assets << ", " << record.current_liabilities << ", "
      << record.inventory << ", " << record.total_assets << ", " << record.total_liabilities << ")";

    return db.exec_direct(sql.str());
  }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
// DatabaseFactory implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////

IFinMartDatabase* DatabaseFactory::create(DatabaseBackend backend, const std::string& connection_string)
{
  switch (backend) 
  {
  case DatabaseBackend::SQLITE:
    return new FinMartSQLite(connection_string);
  case DatabaseBackend::SQLSERVER:
    return new FinMartSQLServer(connection_string);
  default:
    return nullptr;
  }
}

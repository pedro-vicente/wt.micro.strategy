#include "database.hh"
#include "lite.hh"
#include "odbc.hh"
#include <sstream>
#include <ctime>
#include <random>

/////////////////////////////////////////////////////////////////////////////////////////////////////
// FinMartSQLite
// SQLite implementation
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
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
// FinMartSQLServer
// SQL Server ODBC implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////

class FinMartSQLServer : public IFinMartDatabase
{
private:
  odbc db;
  bool connected;

  void initialize_schema()
  {

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // create table if it doesn't exist
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

    table_t check;
    db.fetch("SELECT COUNT(*) as cnt FROM transactions", check);
    if (check.rows.size() > 0 && check.rows[0].col[0] == "0")
    {
      populate_sample_data();
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

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // get_department_spending
  /////////////////////////////////////////////////////////////////////////////////////////////////////

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

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // get_source_system_counts
  /////////////////////////////////////////////////////////////////////////////////////////////////////

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

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // get_total_spending
  /////////////////////////////////////////////////////////////////////////////////////////////////////

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

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // get_backend
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  DatabaseBackend get_backend() const override
  {
    return DatabaseBackend::SQLSERVER;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // get_backend_name
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::string get_backend_name() const override
  {
    return "SQL Server";
  }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
// DatabaseFactory
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

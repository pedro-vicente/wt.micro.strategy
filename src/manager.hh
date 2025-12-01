#ifndef MANAGER_HH
#define MANAGER_HH

#include "db_interface.hh"
#include "api.hh"
#include "get.hh"
#include "metrics.hh"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <sstream>

/////////////////////////////////////////////////////////////////////////////////////////////////////
// DataManager
//
// Central manager class for the ETL pipeline that coordinates:
//   - Database connections (SQLite and SQL Server backends)
//   - MicroStrategy REST API integration
//   - Financial metrics calculation and transformation
//   - Data serialization to JSON format
//
// Usage:
//   DataManager manager;
//   manager.connect_sqlite("finmart.db");
//   manager.connect_mstr("https://mstr.example.com", "user", "pass");
//   manager.set_project("PROJECT_ID");
//   std::vector<FinancialMetrics> metrics = manager.calculate_metrics(records);
//   manager.push_to_dataset(dataset_id, metrics);
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

class DataManager
{
public:
  DataManager();
  ~DataManager();

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Database connection methods
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  int connect_db(DatabaseBackend backend, const std::string& connection_string);
  int connect_sqlite(const std::string& db_path);
  int connect_sqlserver(const std::string& server, const std::string& database,
    const std::string& user = "", const std::string& password = "");
  int disconnect_db();

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // MicroStrategy connection methods
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  int connect_mstr(const std::string& base_url, const std::string& username,
    const std::string& password);
  int disconnect_mstr();
  int set_project(const std::string& project_id);

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Data fetch methods
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  int fetch_financials(const std::string& company_id, std::vector<FinancialRecord>& records);
  int fetch_all_financials(std::vector<FinancialRecord>& records);

  std::vector<transaction> fetch_transactions();
  std::map<std::string, double> fetch_department_spending();
  std::map<std::string, int> fetch_source_system_counts();
  double fetch_total_spending();

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Metrics calculation methods
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::vector<FinancialMetrics> calculate_metrics(const std::vector<FinancialRecord>& records);
  std::vector<double> calculate_yoy_growth(const std::vector<FinancialMetrics>& metrics);

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // MicroStrategy Push API methods
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  int create_dataset(const std::string& name, const std::string& description, std::string& dataset_id);
  int push_to_dataset(const std::string& dataset_id, const std::vector<FinancialMetrics>& metrics);
  int publish_dataset(const std::string& dataset_id);

  int update_cube(const std::string& cube_id, const std::vector<FinancialMetrics>& metrics);

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // JSON serialization methods
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::string metrics_to_json(const std::vector<FinancialMetrics>& metrics);
  std::string dataset_definition_json(const std::string& name, const std::string& description);
  std::string dataset_data_json(const std::vector<FinancialMetrics>& metrics);
  std::string transactions_to_json(const std::vector<transaction>& transactions);

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Accessor methods
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  const Session& get_session() const { return session_; }
  bool is_db_connected() const { return db_ != nullptr && db_->is_open(); }
  bool is_mstr_connected() const { return session_.authenticated; }
  DatabaseBackend get_db_backend() const { return db_ ? db_->get_backend() : DatabaseBackend::SQLITE; }
  std::string get_db_backend_name() const { return db_ ? db_->get_backend_name() : "None"; }

private:
  std::unique_ptr<IFinMartDatabase> db_;
  Session session_;

  void parse_url(const std::string& url, std::string& host, std::string& path);
  void build_auth_headers(std::stringstream& http, const std::string& host);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
// MicroStrategy Push API functions
//
// These are standalone functions that implement the MicroStrategy REST API calls
// for pushing data to datasets and cubes. They are used internally by DataManager
// but can also be called directly with a valid Session object.
//
// Reference: MicroStrategy REST API Documentation
//   https://demo.microstrategy.com/MicroStrategyLibrary/api-docs/
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

int create_dataset(const Session& session, const std::string& json_definition,
  std::string& response);

int create_upload_session(const Session& session, const std::string& dataset_id,
  std::string& upload_id);

int upload_data(const Session& session, const std::string& dataset_id,
  const std::string& upload_id, const std::string& table_name,
  const std::string& json_data, std::string& response);

int publish_dataset(const Session& session, const std::string& dataset_id,
  const std::string& upload_id, std::string& response);

int update_cube_data(const Session& session, const std::string& cube_id,
  const std::string& json_data, std::string& response);

#endif // MANAGER_HH

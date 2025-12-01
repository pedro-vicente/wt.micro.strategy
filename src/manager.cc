#include "manager.hh"
#include "ssl_read.hh"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

/////////////////////////////////////////////////////////////////////////////////////////////////////
// DataManager
/////////////////////////////////////////////////////////////////////////////////////////////////////

DataManager::DataManager() : db_(nullptr)
{
  session_.authenticated = false;
}

DataManager::~DataManager()
{
  disconnect_db();
  if (session_.authenticated)
  {
    disconnect_mstr();
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// connect_db
/////////////////////////////////////////////////////////////////////////////////////////////////////

int DataManager::connect_db(DatabaseBackend backend, const std::string& connection_string)
{
  disconnect_db();
  db_.reset(DatabaseFactory::create(backend, connection_string));
  return (db_ && db_->is_open()) ? 0 : -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// connect_sqlite - convenience method for SQLite
/////////////////////////////////////////////////////////////////////////////////////////////////////

int DataManager::connect_sqlite(const std::string& db_path)
{
  return connect_db(DatabaseBackend::SQLITE, db_path);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// connect_sqlserver
/////////////////////////////////////////////////////////////////////////////////////////////////////

int DataManager::connect_sqlserver(const std::string& server, const std::string& database,
  const std::string& user, const std::string& password)
{
  std::stringstream conn;
  conn << "DRIVER={ODBC Driver 17 for SQL Server};SERVER=" << server
       << ";DATABASE=" << database;

  if (!user.empty())
  {
    conn << ";UID=" << user << ";PWD=" << password;
  }
  else
  {
    conn << ";Trusted_Connection=yes";
  }

  return connect_db(DatabaseBackend::SQLSERVER, conn.str());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// disconnect_db
/////////////////////////////////////////////////////////////////////////////////////////////////////

int DataManager::disconnect_db()
{
  db_.reset();
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// connect_mstr
/////////////////////////////////////////////////////////////////////////////////////////////////////

int DataManager::connect_mstr(const std::string& base_url, const std::string& username,
  const std::string& password)
{
  session_.base_url = base_url;
  session_.username = username;

  if (login(base_url, username, password, session_.auth_token, session_.cookies) == 0)
  {
    session_.authenticated = true;
    return 0;
  }
  return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// disconnect_mstr
/////////////////////////////////////////////////////////////////////////////////////////////////////

int DataManager::disconnect_mstr()
{
  if (session_.authenticated)
  {
    logout(session_.base_url, session_.auth_token, session_.cookies);
    session_.authenticated = false;
    session_.auth_token.clear();
    session_.cookies.clear();
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// set_project
/////////////////////////////////////////////////////////////////////////////////////////////////////

int DataManager::set_project(const std::string& project_id)
{
  session_.project_id = project_id;
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// fetch_transactions
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<transaction> DataManager::fetch_transactions()
{
  if (!is_db_connected())
  {
    return std::vector<transaction>();
  }
  return db_->get_all_transactions();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// fetch_department_spending
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::map<std::string, double> DataManager::fetch_department_spending()
{
  if (!is_db_connected())
  {
    return std::map<std::string, double>();
  }
  return db_->get_department_spending();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// fetch_source_system_counts
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::map<std::string, int> DataManager::fetch_source_system_counts()
{
  if (!is_db_connected())
  {
    return std::map<std::string, int>();
  }
  return db_->get_source_system_counts();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// fetch_total_spending
/////////////////////////////////////////////////////////////////////////////////////////////////////

double DataManager::fetch_total_spending()
{
  if (!is_db_connected())
  {
    return 0.0;
  }
  return db_->get_total_spending();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// fetch_financials
/////////////////////////////////////////////////////////////////////////////////////////////////////

int DataManager::fetch_financials(const std::string& company_id,
  std::vector<FinancialRecord>& records)
{
  records.clear();
  return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// fetch_all_financials
/////////////////////////////////////////////////////////////////////////////////////////////////////

int DataManager::fetch_all_financials(std::vector<FinancialRecord>& records)
{
  records.clear();
  return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// calculate_metrics
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<FinancialMetrics> DataManager::calculate_metrics(
  const std::vector<FinancialRecord>& records)
{
  std::vector<FinancialMetrics> metrics;

  for (size_t idx = 0; idx < records.size(); ++idx)
  {
    const FinancialRecord& rec = records[idx];
    FinancialMetrics m;
    m.period = rec.period;
    m.company_id = rec.company_id;
    m.revenue = rec.revenue;
    m.cogs = rec.cogs;
    m.operating_expenses = rec.operating_expenses;
    m.depreciation = rec.depreciation;
    m.amortization = rec.amortization;
    m.interest = rec.interest;
    m.taxes = rec.taxes;
    m.current_assets = rec.current_assets;
    m.current_liabilities = rec.current_liabilities;
    m.inventory = rec.inventory;
    m.total_assets = rec.total_assets;
    m.total_liabilities = rec.total_liabilities;
    metrics.push_back(m);
  }

  return metrics;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// calculate_yoy_growth
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<double> DataManager::calculate_yoy_growth(
  const std::vector<FinancialMetrics>& metrics)
{
  std::vector<double> growth;
  growth.push_back(0.0); // No prior period for first entry

  for (size_t idx = 1; idx < metrics.size(); ++idx)
  {
    double prev = metrics[idx - 1].revenue;
    double curr = metrics[idx].revenue;
    double yoy = (prev > 0) ? (curr - prev) / prev : 0.0;
    growth.push_back(yoy);
  }

  return growth;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// parse_url - extract host and path from URL
/////////////////////////////////////////////////////////////////////////////////////////////////////

void DataManager::parse_url(const std::string& url, std::string& host, std::string& path)
{
  host = url;
  path = "/";

  size_t pos = host.find("://");
  if (pos != std::string::npos)
  {
    host = host.substr(pos + 3);
  }

  pos = host.find("/");
  if (pos != std::string::npos)
  {
    path = host.substr(pos);
    host = host.substr(0, pos);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// build_auth_headers
/////////////////////////////////////////////////////////////////////////////////////////////////////

void DataManager::build_auth_headers(std::stringstream& http, const std::string& host)
{
  http << "Host: " << host << "\r\n";
  http << "Accept: application/json\r\n";
  http << "X-MSTR-AuthToken: " << session_.auth_token << "\r\n";
  if (!session_.project_id.empty())
  {
    http << "X-MSTR-ProjectID: " << session_.project_id << "\r\n";
  }
  if (!session_.cookies.empty())
  {
    http << "Cookie: " << session_.cookies << "\r\n";
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// dataset_definition_json - JSON schema for MicroStrategy dataset
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string DataManager::dataset_definition_json(const std::string& name,
  const std::string& description)
{
  std::stringstream json;
  json << "{\n";
  json << "  \"name\": \"" << name << "\",\n";
  json << "  \"description\": \"" << description << "\",\n";
  json << "  \"tables\": [{\n";
  json << "    \"name\": \"FinancialMetrics\",\n";
  json << "    \"columnHeaders\": [\n";
  json << "      {\"name\": \"Period\", \"dataType\": \"STRING\"},\n";
  json << "      {\"name\": \"CompanyID\", \"dataType\": \"STRING\"},\n";
  json << "      {\"name\": \"Revenue\", \"dataType\": \"DOUBLE\"},\n";
  json << "      {\"name\": \"GrossProfit\", \"dataType\": \"DOUBLE\"},\n";
  json << "      {\"name\": \"GrossMargin\", \"dataType\": \"DOUBLE\"},\n";
  json << "      {\"name\": \"EBITDA\", \"dataType\": \"DOUBLE\"},\n";
  json << "      {\"name\": \"EBIT\", \"dataType\": \"DOUBLE\"},\n";
  json << "      {\"name\": \"NetIncome\", \"dataType\": \"DOUBLE\"},\n";
  json << "      {\"name\": \"NetMargin\", \"dataType\": \"DOUBLE\"},\n";
  json << "      {\"name\": \"WorkingCapital\", \"dataType\": \"DOUBLE\"},\n";
  json << "      {\"name\": \"CurrentRatio\", \"dataType\": \"DOUBLE\"},\n";
  json << "      {\"name\": \"QuickRatio\", \"dataType\": \"DOUBLE\"},\n";
  json << "      {\"name\": \"DebtToEquity\", \"dataType\": \"DOUBLE\"},\n";
  json << "      {\"name\": \"ROA\", \"dataType\": \"DOUBLE\"},\n";
  json << "      {\"name\": \"ROE\", \"dataType\": \"DOUBLE\"}\n";
  json << "    ]\n";
  json << "  }]\n";
  json << "}";
  return json.str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// dataset_data_json
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string DataManager::dataset_data_json(const std::vector<FinancialMetrics>& metrics)
{
  std::stringstream json;
  json << std::fixed << std::setprecision(4);
  json << "{\n";
  json << "  \"data\": [\n";

  for (size_t idx = 0; idx < metrics.size(); ++idx)
  {
    const FinancialMetrics& m = metrics[idx];
    json << "    [\"" << m.period << "\", \"" << m.company_id << "\", "
         << m.revenue << ", " << m.gross_profit() << ", " << m.gross_margin() << ", "
         << m.ebitda() << ", " << m.ebit() << ", " << m.net_income() << ", "
         << m.net_margin() << ", " << m.working_capital() << ", "
         << m.current_ratio() << ", " << m.quick_ratio() << ", "
         << m.debt_to_equity() << ", " << m.return_on_assets() << ", "
         << m.return_on_equity() << "]";
    if (idx < metrics.size() - 1) json << ",";
    json << "\n";
  }

  json << "  ]\n";
  json << "}";
  return json.str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// metrics_to_json
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string DataManager::metrics_to_json(const std::vector<FinancialMetrics>& metrics)
{
  std::stringstream json;
  json << std::fixed << std::setprecision(4);
  json << "{\n";
  json << "  \"metrics\": [\n";

  for (size_t idx = 0; idx < metrics.size(); ++idx)
  {
    const FinancialMetrics& m = metrics[idx];
    json << "    {\n";
    json << "      \"period\": \"" << m.period << "\",\n";
    json << "      \"company_id\": \"" << m.company_id << "\",\n";
    json << "      \"revenue\": " << m.revenue << ",\n";
    json << "      \"gross_profit\": " << m.gross_profit() << ",\n";
    json << "      \"gross_margin\": " << m.gross_margin() << ",\n";
    json << "      \"ebitda\": " << m.ebitda() << ",\n";
    json << "      \"ebit\": " << m.ebit() << ",\n";
    json << "      \"net_income\": " << m.net_income() << ",\n";
    json << "      \"net_margin\": " << m.net_margin() << ",\n";
    json << "      \"operating_margin\": " << m.operating_margin() << ",\n";
    json << "      \"working_capital\": " << m.working_capital() << ",\n";
    json << "      \"current_ratio\": " << m.current_ratio() << ",\n";
    json << "      \"quick_ratio\": " << m.quick_ratio() << ",\n";
    json << "      \"debt_to_equity\": " << m.debt_to_equity() << ",\n";
    json << "      \"debt_ratio\": " << m.debt_ratio() << ",\n";
    json << "      \"roa\": " << m.return_on_assets() << ",\n";
    json << "      \"roe\": " << m.return_on_equity() << "\n";
    json << "    }";
    if (idx < metrics.size() - 1) json << ",";
    json << "\n";
  }

  json << "  ]\n";
  json << "}";
  return json.str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// transactions_to_json
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string DataManager::transactions_to_json(const std::vector<transaction>& transactions)
{
  std::stringstream json;
  json << std::fixed << std::setprecision(2);
  json << "{\n";
  json << "  \"transactions\": [\n";

  for (size_t idx = 0; idx < transactions.size(); ++idx)
  {
    const transaction& t = transactions[idx];
    json << "    {\n";
    json << "      \"id\": " << t.id << ",\n";
    json << "      \"date\": \"" << t.date << "\",\n";
    json << "      \"department\": \"" << t.department << "\",\n";
    json << "      \"category\": \"" << t.category << "\",\n";
    json << "      \"vendor\": \"" << t.vendor << "\",\n";
    json << "      \"amount\": " << t.amount << ",\n";
    json << "      \"status\": \"" << t.status << "\",\n";
    json << "      \"source_system\": \"" << t.source_system << "\"\n";
    json << "    }";
    if (idx < transactions.size() - 1) json << ",";
    json << "\n";
  }

  json << "  ]\n";
  json << "}";
  return json.str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// create_dataset
/////////////////////////////////////////////////////////////////////////////////////////////////////

int DataManager::create_dataset(const std::string& name, const std::string& description,
  std::string& dataset_id)
{
  if (!session_.authenticated) return -1;

  std::string json_def = dataset_definition_json(name, description);
  std::string response;

  if (::create_dataset(session_, json_def, response) != 0)
  {
    return -1;
  }

  dataset_id = extract_value(response, "id");
  return dataset_id.empty() ? -1 : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// push_to_dataset
/////////////////////////////////////////////////////////////////////////////////////////////////////

int DataManager::push_to_dataset(const std::string& dataset_id,
  const std::vector<FinancialMetrics>& metrics)
{
  if (!session_.authenticated) return -1;

  std::string upload_id;
  if (create_upload_session(session_, dataset_id, upload_id) != 0)
  {
    return -1;
  }

  std::string json_data = dataset_data_json(metrics);
  std::string response;
  if (upload_data(session_, dataset_id, upload_id, "FinancialMetrics",
    json_data, response) != 0)
  {
    return -1;
  }

  return ::publish_dataset(session_, dataset_id, upload_id, response);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// publish_dataset
/////////////////////////////////////////////////////////////////////////////////////////////////////

int DataManager::publish_dataset(const std::string& dataset_id)
{
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// update_cube
/////////////////////////////////////////////////////////////////////////////////////////////////////

int DataManager::update_cube(const std::string& cube_id,
  const std::vector<FinancialMetrics>& metrics)
{
  if (!session_.authenticated) return -1;

  std::string json_data = dataset_data_json(metrics);
  std::string response;
  return update_cube_data(session_, cube_id, json_data, response);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// MicroStrategy Push API 
/////////////////////////////////////////////////////////////////////////////////////////////////////

int create_dataset(const Session& session, const std::string& json_definition,
  std::string& response)
{
  std::string host = session.base_url;
  std::string path = "/api/datasets";

  size_t pos = host.find("://");
  if (pos != std::string::npos)
  {
    host = host.substr(pos + 3);
  }

  pos = host.find("/");
  if (pos != std::string::npos)
  {
    path = host.substr(pos) + "/api/datasets";
    host = host.substr(0, pos);
  }

  const std::string port_num = "443";

  std::stringstream http;
  http << "POST " << path << " HTTP/1.1\r\n";
  http << "Host: " << host << "\r\n";
  http << "Accept: application/json\r\n";
  http << "Content-Type: application/json\r\n";
  http << "X-MSTR-AuthToken: " << session.auth_token << "\r\n";
  if (!session.project_id.empty())
  {
    http << "X-MSTR-ProjectID: " << session.project_id << "\r\n";
  }
  if (!session.cookies.empty())
  {
    http << "Cookie: " << session.cookies << "\r\n";
  }
  http << "Content-Length: " << json_definition.length() << "\r\n";
  http << "Connection: close\r\n\r\n";
  http << json_definition;

  std::vector<std::string> headers;
  return ssl_read(host, port_num, http.str(), response, headers);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// create_upload_session
/////////////////////////////////////////////////////////////////////////////////////////////////////

int create_upload_session(const Session& session, const std::string& dataset_id, std::string& upload_id)
{
  std::string host = session.base_url;
  std::string path = "/api/datasets/" + dataset_id + "/uploadSessions";

  size_t pos = host.find("://");
  if (pos != std::string::npos)
  {
    host = host.substr(pos + 3);
  }

  pos = host.find("/");
  if (pos != std::string::npos)
  {
    path = host.substr(pos) + "/api/datasets/" + dataset_id + "/uploadSessions";
    host = host.substr(0, pos);
  }

  const std::string port_num = "443";

  std::stringstream http;
  http << "POST " << path << " HTTP/1.1\r\n";
  http << "Host: " << host << "\r\n";
  http << "Accept: application/json\r\n";
  http << "Content-Type: application/json\r\n";
  http << "X-MSTR-AuthToken: " << session.auth_token << "\r\n";
  if (!session.project_id.empty())
  {
    http << "X-MSTR-ProjectID: " << session.project_id << "\r\n";
  }
  if (!session.cookies.empty())
  {
    http << "Cookie: " << session.cookies << "\r\n";
  }
  http << "Content-Length: 0\r\n";
  http << "Connection: close\r\n\r\n";

  std::string response;
  std::vector<std::string> headers;
  if (ssl_read(host, port_num, http.str(), response, headers) != 0)
  {
    return -1;
  }

  upload_id = extract_value(response, "uploadSessionId");
  return upload_id.empty() ? -1 : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// upload_data
/////////////////////////////////////////////////////////////////////////////////////////////////////

int upload_data(const Session& session, const std::string& dataset_id,
  const std::string& upload_id, const std::string& table_name,
  const std::string& json_data, std::string& response)
{
  std::string host = session.base_url;
  std::string path = "/api/datasets/" + dataset_id + "/uploadSessions/" + upload_id +
    "?tableName=" + table_name;

  size_t pos = host.find("://");
  if (pos != std::string::npos)
  {
    host = host.substr(pos + 3);
  }

  pos = host.find("/");
  if (pos != std::string::npos)
  {
    std::string base_path = host.substr(pos);
    host = host.substr(0, pos);
    path = base_path + "/api/datasets/" + dataset_id + "/uploadSessions/" + upload_id +
      "?tableName=" + table_name;
  }

  const std::string port_num = "443";

  std::stringstream http;
  http << "PUT " << path << " HTTP/1.1\r\n";
  http << "Host: " << host << "\r\n";
  http << "Accept: application/json\r\n";
  http << "Content-Type: application/json\r\n";
  http << "X-MSTR-AuthToken: " << session.auth_token << "\r\n";
  if (!session.project_id.empty())
  {
    http << "X-MSTR-ProjectID: " << session.project_id << "\r\n";
  }
  if (!session.cookies.empty())
  {
    http << "Cookie: " << session.cookies << "\r\n";
  }
  http << "Content-Length: " << json_data.length() << "\r\n";
  http << "Connection: close\r\n\r\n";
  http << json_data;

  std::vector<std::string> headers;
  return ssl_read(host, port_num, http.str(), response, headers);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// publish_dataset
/////////////////////////////////////////////////////////////////////////////////////////////////////

int publish_dataset(const Session& session, const std::string& dataset_id,
  const std::string& upload_id, std::string& response)
{
  std::string host = session.base_url;
  std::string path = "/api/datasets/" + dataset_id + "/uploadSessions/" + upload_id + "/publish";

  size_t pos = host.find("://");
  if (pos != std::string::npos)
  {
    host = host.substr(pos + 3);
  }

  pos = host.find("/");
  if (pos != std::string::npos)
  {
    std::string base_path = host.substr(pos);
    host = host.substr(0, pos);
    path = base_path + "/api/datasets/" + dataset_id + "/uploadSessions/" + upload_id + "/publish";
  }

  const std::string port_num = "443";

  std::stringstream http;
  http << "POST " << path << " HTTP/1.1\r\n";
  http << "Host: " << host << "\r\n";
  http << "Accept: application/json\r\n";
  http << "X-MSTR-AuthToken: " << session.auth_token << "\r\n";
  if (!session.project_id.empty())
  {
    http << "X-MSTR-ProjectID: " << session.project_id << "\r\n";
  }
  if (!session.cookies.empty())
  {
    http << "Cookie: " << session.cookies << "\r\n";
  }
  http << "Content-Length: 0\r\n";
  http << "Connection: close\r\n\r\n";

  std::vector<std::string> headers;
  return ssl_read(host, port_num, http.str(), response, headers);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// update_cube_data
/////////////////////////////////////////////////////////////////////////////////////////////////////

int update_cube_data(const Session& session, const std::string& cube_id,
  const std::string& json_data, std::string& response)
{
  std::string host = session.base_url;
  std::string path = "/api/cubes/" + cube_id + "/instances";

  size_t pos = host.find("://");
  if (pos != std::string::npos)
  {
    host = host.substr(pos + 3);
  }

  pos = host.find("/");
  if (pos != std::string::npos)
  {
    std::string base_path = host.substr(pos);
    host = host.substr(0, pos);
    path = base_path + "/api/cubes/" + cube_id + "/instances";
  }

  const std::string port_num = "443";

  std::stringstream http;
  http << "POST " << path << " HTTP/1.1\r\n";
  http << "Host: " << host << "\r\n";
  http << "Accept: application/json\r\n";
  http << "Content-Type: application/json\r\n";
  http << "X-MSTR-AuthToken: " << session.auth_token << "\r\n";
  if (!session.project_id.empty())
  {
    http << "X-MSTR-ProjectID: " << session.project_id << "\r\n";
  }
  if (!session.cookies.empty())
  {
    http << "Cookie: " << session.cookies << "\r\n";
  }
  http << "Content-Length: " << json_data.length() << "\r\n";
  http << "Connection: close\r\n\r\n";
  http << json_data;

  std::vector<std::string> headers;
  return ssl_read(host, port_num, http.str(), response, headers);
}


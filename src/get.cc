#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include "ssl_read.hh"
#include "get.hh"

/////////////////////////////////////////////////////////////////////////////////////////////////////
// extract_value
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string extract_value(const std::string& content, const std::string& key)
{
  size_t pos_key = content.find("\"" + key + "\"");
  if (pos_key == std::string::npos) return "";

  size_t pos_colon = content.find(":", pos_key);
  if (pos_colon == std::string::npos) return "";

  size_t first = content.find("\"", pos_colon);
  if (first == std::string::npos) return "";

  size_t second = content.find("\"", first + 1);
  if (second == std::string::npos) return "";

  return content.substr(first + 1, second - first - 1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// extract_header_value
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string extract_header_value(const std::vector<std::string>& headers, const std::string& key)
{
  std::string key_lower = key;
  std::transform(key_lower.begin(), key_lower.end(), key_lower.begin(), ::tolower);

  for (size_t idx = 0; idx < headers.size(); idx++)
  {
    const std::string& header = headers[idx];
    std::string header_lower = header;
    std::transform(header_lower.begin(), header_lower.end(), header_lower.begin(), ::tolower);

    size_t pos = header_lower.find(key_lower + ":");
    if (pos != std::string::npos)
    {
      std::string value = header.substr(pos + key.length() + 1);
      value.erase(0, value.find_first_not_of(" \t\r\n"));
      value.erase(value.find_last_not_of(" \t\r\n") + 1);
      return value;
    }
  }
  return "";
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// login
// Authenticate with MicroStrategy REST API
// POST https://{base_url}/api/auth/login HTTP/1.1
// Content-Type: application/json
//
// Body: {"username":"administrator","password":"yourPassword"}
//
// Returns:
//   auth_token - X-MSTR-AuthToken from response header
//   cookies    - Session cookies (JSESSIONID, etc.)
/////////////////////////////////////////////////////////////////////////////////////////////////////

int login(const std::string& base_url, const std::string& username, const std::string& password,
  std::string& auth_token, std::string& cookies)
{
  std::string host = base_url;
  std::string path = "/api/auth/login";

  size_t pos = host.find("://");
  if (pos != std::string::npos)
  {
    host = host.substr(pos + 3);
  }

  pos = host.find("/");
  if (pos != std::string::npos)
  {
    path = host.substr(pos) + "/api/auth/login";
    host = host.substr(0, pos);
  }

  const std::string port_num = "443";

  std::stringstream json_body;
  json_body << "{\"username\":\"" << username << "\",\"password\":\"" << password << "\"}";
  std::string body = json_body.str();

  std::stringstream http;
  http << "POST " << path << " HTTP/1.1\r\n";
  http << "Host: " << host << "\r\n";
  http << "Accept: application/json\r\n";
  http << "Content-Type: application/json\r\n";
  http << "Content-Length: " << body.length() << "\r\n";
  http << "Connection: close\r\n\r\n";
  http << body;

  std::cout << "Request:\n" << http.str() << std::endl;

  std::string response;
  std::vector<std::string> headers;
  ssl_read(host, port_num, http.str(), response, headers);

  auth_token = extract_header_value(headers, "X-MSTR-AuthToken");

  cookies.clear();
  for (size_t idx = 0; idx < headers.size(); idx++)
  {
    const std::string& header = headers[idx];
    std::string header_lower = header;
    std::transform(header_lower.begin(), header_lower.end(), header_lower.begin(), ::tolower);

    if (header_lower.find("set-cookie:") != std::string::npos)
    {
      size_t pos = header.find(":");
      if (pos != std::string::npos)
      {
        std::string cookie_value = header.substr(pos + 1);
        cookie_value.erase(0, cookie_value.find_first_not_of(" \t"));
        size_t semicolon = cookie_value.find(";");
        if (semicolon != std::string::npos)
        {
          cookie_value = cookie_value.substr(0, semicolon);
        }
        cookie_value.erase(cookie_value.find_last_not_of(" \t\r\n") + 1);

        if (!cookie_value.empty())
        {
          if (!cookies.empty()) cookies += "; ";
          cookies += cookie_value;
        }
      }
    }
  }

  if (auth_token.empty())
  {
    std::cerr << "Login failed: no auth token received" << std::endl;
    return -1;
  }

  std::cout << "Auth Token: " << auth_token << std::endl;
  std::cout << "Cookies: " << cookies << std::endl;

  if (!response.empty())
  {
    std::ofstream ofs("login_response.json");
    ofs << response;
    ofs.close();
  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// get_projects
// get list of projects user has access to
// GET https://{base_url}/api/projects HTTP/1.1
// X-MSTR-AuthToken: {auth_token}
// Cookie: {cookies}
/////////////////////////////////////////////////////////////////////////////////////////////////////

int get_projects(const std::string& base_url, const std::string& auth_token, const std::string& cookies)
{
  std::string host = base_url;
  std::string path = "/api/projects";

  size_t pos = host.find("://");
  if (pos != std::string::npos)
  {
    host = host.substr(pos + 3);
  }

  pos = host.find("/");
  if (pos != std::string::npos)
  {
    path = host.substr(pos) + "/api/projects";
    host = host.substr(0, pos);
  }

  const std::string port_num = "443";

  std::stringstream http;
  http << "GET " << path << " HTTP/1.1\r\n";
  http << "Host: " << host << "\r\n";
  http << "Accept: application/json\r\n";
  http << "X-MSTR-AuthToken: " << auth_token << "\r\n";
  if (!cookies.empty())
  {
    http << "Cookie: " << cookies << "\r\n";
  }
  http << "Connection: close\r\n\r\n";

  std::cout << "Request:\n" << http.str() << std::endl;

  std::string response;
  std::vector<std::string> headers;
  ssl_read(host, port_num, http.str(), response, headers);

  if (!response.size())
  {
    return -1;
  }

  std::cout << response << std::endl;

  std::ofstream ofs("projects.json");
  ofs << response;
  ofs.close();

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// get_report_definition
// GET https://{base_url}/api/model/reports/{reportId}?showExpressionAs=tree HTTP/1.1
// X-MSTR-AuthToken: {auth_token}
// X-MSTR-ProjectID: {project_id}
// Cookie: {cookies}
/////////////////////////////////////////////////////////////////////////////////////////////////////

int get_report_definition(const std::string& base_url, const std::string& auth_token,
  const std::string& cookies, const std::string& project_id,
  const std::string& report_id)
{
  std::string host = base_url;
  std::string path = "/api/model/reports/" + report_id + "?showExpressionAs=tree";

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
    path = base_path + "/api/model/reports/" + report_id + "?showExpressionAs=tree";
  }

  const std::string port_num = "443";

  std::stringstream http;
  http << "GET " << path << " HTTP/1.1\r\n";
  http << "Host: " << host << "\r\n";
  http << "Accept: application/json\r\n";
  http << "X-MSTR-AuthToken: " << auth_token << "\r\n";
  http << "X-MSTR-ProjectID: " << project_id << "\r\n";
  if (!cookies.empty())
  {
    http << "Cookie: " << cookies << "\r\n";
  }
  http << "Connection: close\r\n\r\n";

  std::cout << "Request:\n" << http.str() << std::endl;

  std::string response;
  std::vector<std::string> headers;
  ssl_read(host, port_num, http.str(), response, headers);

  if (!response.size())
  {
    return -1;
  }

  std::cout << response << std::endl;

  std::ofstream ofs("report_" + report_id + ".json");
  ofs << response;
  ofs.close();

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// create_cube
// Create a cube instance for data retrieval
// POST https://{base_url}/api/cubes/{cubeId}/instances HTTP/1.1
// X-MSTR-AuthToken: {auth_token}
// X-MSTR-ProjectID: {project_id}
// Cookie: {cookies}
/////////////////////////////////////////////////////////////////////////////////////////////////////

int create_cube(const std::string& base_url, const std::string& auth_token,
  const std::string& cookies, const std::string& project_id,
  const std::string& cube_id)
{
  std::string host = base_url;
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
  http << "X-MSTR-AuthToken: " << auth_token << "\r\n";
  http << "X-MSTR-ProjectID: " << project_id << "\r\n";
  if (!cookies.empty())
  {
    http << "Cookie: " << cookies << "\r\n";
  }
  http << "Content-Length: 0\r\n";
  http << "Connection: close\r\n\r\n";

  std::cout << "Request:\n" << http.str() << std::endl;

  std::string response;
  std::vector<std::string> headers;
  ssl_read(host, port_num, http.str(), response, headers);

  if (!response.size())
  {
    return -1;
  }

  std::cout << response << std::endl;

  std::ofstream ofs("cube_instance_" + cube_id + ".json");
  ofs << response;
  ofs.close();

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// logout
// Log out and terminate the session
// POST https://{base_url}/api/auth/logout HTTP/1.1
// X-MSTR-AuthToken: {auth_token}
// Cookie: {cookies}
/////////////////////////////////////////////////////////////////////////////////////////////////////

int logout(const std::string& base_url, const std::string& auth_token, const std::string& cookies)
{
  std::string host = base_url;
  std::string path = "/api/auth/logout";

  size_t pos = host.find("://");
  if (pos != std::string::npos)
  {
    host = host.substr(pos + 3);
  }

  pos = host.find("/");
  if (pos != std::string::npos)
  {
    path = host.substr(pos) + "/api/auth/logout";
    host = host.substr(0, pos);
  }

  const std::string port_num = "443";

  std::stringstream http;
  http << "POST " << path << " HTTP/1.1\r\n";
  http << "Host: " << host << "\r\n";
  http << "Accept: application/json\r\n";
  http << "X-MSTR-AuthToken: " << auth_token << "\r\n";
  if (!cookies.empty())
  {
    http << "Cookie: " << cookies << "\r\n";
  }
  http << "Content-Length: 0\r\n";
  http << "Connection: close\r\n\r\n";

  std::cout << "Logout request sent" << std::endl;

  std::string response;
  std::vector<std::string> headers;
  ssl_read(host, port_num, http.str(), response, headers);

  return 0;
}

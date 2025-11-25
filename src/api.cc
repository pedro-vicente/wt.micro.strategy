#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include "ssl_read.hh"
#include "get.hh"
#include "api.hh"

/////////////////////////////////////////////////////////////////////////////////////////////////////
// build_auth_header
// build HTTP headers with auth
/////////////////////////////////////////////////////////////////////////////////////////////////////

static void build_auth_headers(std::stringstream& http, const Session& session,
  const std::string& host)
{
  http << "Host: " << host << "\r\n";
  http << "Accept: application/json\r\n";
  http << "X-MSTR-AuthToken: " << session.authToken << "\r\n";
  if (!session.projectId.empty())
  {
    http << "X-MSTR-ProjectID: " << session.projectId << "\r\n";
  }
  if (!session.cookies.empty())
  {
    http << "Cookie: " << session.cookies << "\r\n";
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// parse_url
// parse host and path from base URL
/////////////////////////////////////////////////////////////////////////////////////////////////////

static void parse_url(const std::string& base_url, std::string& host, std::string& base_path)
{
  host = base_url;
  base_path = "";

  size_t pos = host.find("://");
  if (pos != std::string::npos)
  {
    host = host.substr(pos + 3);
  }

  pos = host.find("/");
  if (pos != std::string::npos)
  {
    base_path = host.substr(pos);
    host = host.substr(0, pos);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// search
// search for objects by name
// GET /api/searches/results?name={name}&type={type}&limit={limit}
/////////////////////////////////////////////////////////////////////////////////////////////////////

int search(const Session& session, const std::string& name,
  int type, int limit, std::string& response)
{
  std::string host, base_path;
  parse_url(session.baseUrl, host, base_path);

  std::stringstream path;
  path << base_path << "/api/searches/results?";
  if (!name.empty())
  {
    path << "name=" << name << "&";
  }
  if (type > 0)
  {
    path << "type=" << type << "&";
  }
  path << "limit=" << limit;

  const std::string port_num = "443";

  std::stringstream http;
  http << "GET " << path.str() << " HTTP/1.1\r\n";
  build_auth_headers(http, session, host);
  http << "Connection: close\r\n\r\n";

  std::vector<std::string> headers;
  return ssl_read(host, port_num, http.str(), response, headers);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// get_library
// get user's library items (dossiers, documents)
// GET /api/library?limit={limit}
/////////////////////////////////////////////////////////////////////////////////////////////////////

int get_library(const Session& session, int limit, std::string& response)
{
  std::string host, base_path;
  parse_url(session.baseUrl, host, base_path);

  std::stringstream path;
  path << base_path << "/api/library?outputFlag=DEFAULT&limit=" << limit;

  const std::string port_num = "443";

  std::stringstream http;
  http << "GET " << path.str() << " HTTP/1.1\r\n";
  build_auth_headers(http, session, host);
  http << "Connection: close\r\n\r\n";

  std::vector<std::string> headers;
  return ssl_read(host, port_num, http.str(), response, headers);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// get_report
// execute a report and get data
// POST /api/reports/{reportId}/instances
/////////////////////////////////////////////////////////////////////////////////////////////////////

int get_report(const Session& session, const std::string& report_id,
  std::string& response)
{
  std::string host, base_path;
  parse_url(session.baseUrl, host, base_path);

  std::string path = base_path + "/api/reports/" + report_id + "/instances?limit=100";

  const std::string port_num = "443";

  std::stringstream http;
  http << "POST " << path << " HTTP/1.1\r\n";
  build_auth_headers(http, session, host);
  http << "Content-Length: 0\r\n";
  http << "Connection: close\r\n\r\n";

  std::vector<std::string> headers;
  return ssl_read(host, port_num, http.str(), response, headers);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// get_cube
// get cube instance data with pagination
// GET /api/cubes/{cubeId}/instances/{instanceId}?offset={offset}&limit={limit}
/////////////////////////////////////////////////////////////////////////////////////////////////////

int get_cube(const Session& session, const std::string& cube_id,
  const std::string& instance_id, int offset, int limit,
  std::string& response)
{
  std::string host, base_path;
  parse_url(session.baseUrl, host, base_path);

  std::stringstream path;
  path << base_path << "/api/cubes/" << cube_id << "/instances/" << instance_id;
  path << "?offset=" << offset << "&limit=" << limit;

  const std::string port_num = "443";

  std::stringstream http;
  http << "GET " << path.str() << " HTTP/1.1\r\n";
  build_auth_headers(http, session, host);
  http << "Connection: close\r\n\r\n";

  std::vector<std::string> headers;
  return ssl_read(host, port_num, http.str(), response, headers);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// get_dossiers
// get list of dossiers (dashboards)
// GET /api/searches/results?type=55&limit=50
/////////////////////////////////////////////////////////////////////////////////////////////////////

int get_dossiers(const Session& session, std::string& response)
{
  return search(session, "", MSTR_DOSSIER, 50, response);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// extract_json_array
/////////////////////////////////////////////////////////////////////////////////////////////////////

static std::vector<std::string> extract_json_array(const std::string& json)
{
  std::vector<std::string> items;
  size_t pos = json.find('[');
  if (pos == std::string::npos) return items;

  int depth = 0;
  size_t start = 0;
  bool in_string = false;
  char prev_char = 0;

  for (size_t i = pos; i < json.size(); i++)
  {
    char c = json[i];

    // Handle string escaping
    if (c == '"' && prev_char != '\\')
    {
      in_string = !in_string;
    }

    if (!in_string)
    {
      if (c == '{')
      {
        if (depth == 0) start = i;
        depth++;
      }
      else if (c == '}')
      {
        depth--;
        if (depth == 0)
        {
          items.push_back(json.substr(start, i - start + 1));
        }
      }
    }
    prev_char = c;
  }

  return items;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// parse_projects
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<Project> parse_projects(const std::string& json)
{
  std::vector<Project> projects;
  std::vector<std::string> items = extract_json_array(json);

  for (size_t idx = 0; idx < items.size(); idx++)
  {
    const std::string& item = items[idx];
    Project proj;
    proj.id = extract_value(item, "id");
    proj.name = extract_value(item, "name");
    proj.description = extract_value(item, "description");
    proj.status = extract_value(item, "status");
    if (!proj.id.empty())
    {
      projects.push_back(proj);
    }
  }

  return projects;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// parse_search_results
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<SearchResult> parse_search_results(const std::string& json)
{
  std::vector<SearchResult> results;

  size_t result_pos = json.find("\"result\"");
  if (result_pos == std::string::npos)
  {
    std::vector<std::string> items = extract_json_array(json);
    for (size_t idx = 0; idx < items.size(); idx++)
    {
      const std::string& item = items[idx];
      SearchResult sr;
      sr.id = extract_value(item, "id");
      sr.name = extract_value(item, "name");
      sr.type = extract_value(item, "type");
      sr.subtype = extract_value(item, "subtype");
      sr.dateModified = extract_value(item, "dateModified");
      sr.owner = extract_value(item, "owner");
      if (!sr.id.empty())
      {
        results.push_back(sr);
      }
    }
    return results;
  }

  std::string result_section = json.substr(result_pos);
  std::vector<std::string> items = extract_json_array(result_section);

  for (size_t idx = 0; idx < items.size(); idx++)
  {
    const std::string& item = items[idx];
    SearchResult sr;
    sr.id = extract_value(item, "id");
    sr.name = extract_value(item, "name");
    sr.type = extract_value(item, "type");
    sr.subtype = extract_value(item, "subtype");
    sr.dateModified = extract_value(item, "dateModified");
    sr.owner = extract_value(item, "owner");
    if (!sr.id.empty())
    {
      results.push_back(sr);
    }
  }

  return results;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// parse_library_items
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<LibraryItem> parse_library_items(const std::string& json)
{
  std::vector<LibraryItem> items_list;
  std::vector<std::string> items = extract_json_array(json);

  for (size_t idx = 0; idx < items.size(); idx++)
  {
    const std::string& item = items[idx];
    LibraryItem li;
    li.id = extract_value(item, "id");
    li.name = extract_value(item, "name");
    li.type = extract_value(item, "type");
    li.projectId = extract_value(item, "projectId");
    li.dateModified = extract_value(item, "dateModified");
    if (!li.id.empty())
    {
      items_list.push_back(li);
    }
  }

  return items_list;
}

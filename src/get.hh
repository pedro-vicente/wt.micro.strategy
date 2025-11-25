#ifndef GET_MSTR_HH
#define GET_MSTR_HH

#include <string>
#include <vector>

/////////////////////////////////////////////////////////////////////////////////////////////////////
// REST API functions
/////////////////////////////////////////////////////////////////////////////////////////////////////

int login(const std::string& base_url, const std::string& username, const std::string& password,
  std::string& auth_token, std::string& cookies);
int get_projects(const std::string& base_url, const std::string& auth_token, const std::string& cookies);
int get_report_definition(const std::string& base_url, const std::string& auth_token,
  const std::string& cookies, const std::string& project_id,
  const std::string& report_id);
int create_cube(const std::string& base_url, const std::string& auth_token,
  const std::string& cookies, const std::string& project_id,
  const std::string& cube_id);
int logout(const std::string& base_url, const std::string& auth_token, const std::string& cookies);

std::string extract_value(const std::string& content, const std::string& key);
std::string extract_header_value(const std::vector<std::string>& headers, const std::string& key);

#endif

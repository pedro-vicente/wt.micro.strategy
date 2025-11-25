#include <iostream>
#include <sstream>
#include <fstream>
#include "get.hh"

/////////////////////////////////////////////////////////////////////////////////////////////////////
// main - MicroStrategy REST API example
/////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Load configuration from file
  // config.json should contain:
  // {
  //   "MSTR_BASE_URL": "demo.microstrategy.com/MicroStrategyLibrary",
  //   "MSTR_USERNAME": "administrator",
  //   "MSTR_PASSWORD": "yourPassword",
  //   "MSTR_PROJECT_ID": "B7CA92F04B9FAE8D941C3E9B7E0CD754",
  //   "MSTR_REPORT_ID": "FFDAB82F4CA397073ABD4196FCBCD918",
  //   "MSTR_CUBE_ID": "your_cube_id_here"
  // }
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::ifstream file("config.json");
  if (!file.is_open())
  {
    std::cerr << "Error: Cannot open config.json" << std::endl;
    std::cerr << "Please create config.json with your MicroStrategy credentials" << std::endl;
    return -1;
  }

  std::stringstream ss;
  ss << file.rdbuf();
  std::string buf = ss.str();
  file.close();

  std::string base_url = extract_value(buf, "MSTR_BASE_URL");
  std::string username = extract_value(buf, "MSTR_USERNAME");
  std::string password = extract_value(buf, "MSTR_PASSWORD");
  std::string project_id = extract_value(buf, "MSTR_PROJECT_ID");
  std::string report_id = extract_value(buf, "MSTR_REPORT_ID");
  std::string cube_id = extract_value(buf, "MSTR_CUBE_ID");

  if (base_url.empty() || username.empty())
  {
    std::cerr << "Error: Missing required configuration values" << std::endl;
    return -1;
  }

  std::cout << "=== MicroStrategy REST API Example ===" << std::endl;
  std::cout << "Base URL: " << base_url << std::endl;
  std::cout << "Username: " << username << std::endl << std::endl;

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Step 1: Login to get authentication token
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::cout << "Step 1: Logging in..." << std::endl;
  std::string auth_token;
  std::string cookies;
  
  int result = mstr_login(base_url, username, password, auth_token, cookies);
  if (result != 0)
  {
    std::cerr << "Login failed" << std::endl;
    return -1;
  }
  std::cout << "Login successful!\n" << std::endl;

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Step 2: Get list of projects
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::cout << "Step 2: Getting projects list..." << std::endl;
  result = mstr_get_projects(base_url, auth_token, cookies);
  if (result != 0)
  {
    std::cerr << "Failed to get projects" << std::endl;
  }
  std::cout << "Projects retrieved!\n" << std::endl;

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Step 3: Get report definition (if report_id provided)
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  if (!project_id.empty() && !report_id.empty())
  {
    std::cout << "Step 3: Getting report definition..." << std::endl;
    result = mstr_get_report_definition(base_url, auth_token, cookies, project_id, report_id);
    if (result != 0)
    {
      std::cerr << "Failed to get report definition" << std::endl;
    }
    else
    {
      std::cout << "Report definition retrieved!\n" << std::endl;
    }
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Step 4: Create cube instance (if cube_id provided)
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  if (!project_id.empty() && !cube_id.empty())
  {
    std::cout << "Step 4: Creating cube instance..." << std::endl;
    result = mstr_create_cube_instance(base_url, auth_token, cookies, project_id, cube_id);
    if (result != 0)
    {
      std::cerr << "Failed to create cube instance" << std::endl;
    }
    else
    {
      std::cout << "Cube instance created!\n" << std::endl;
    }
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // Step 5: Logout
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::cout << "Step 5: Logging out..." << std::endl;
  mstr_logout(base_url, auth_token, cookies);
  std::cout << "Logged out successfully!" << std::endl;

  std::cout << "\n=== All operations completed ===" << std::endl;
  return 0;
}

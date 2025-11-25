# MicroStrategy REST API C++ Client

A C++ client library for interacting with the MicroStrategy REST API using native SSL/TLS connections via ASIO.

## Overview

This client provides an interface to authenticate, query projects, retrieve reports, and manipulate cubes on MicroStrategy servers. 

**MicroStrategy REST API Reference** [https://microstrategy.github.io/rest-api-docs](https://microstrategy.github.io/rest-api-docs/)

## Configuration

Create a `config.json` file in the working directory:

```json
{
  "MSTR_BASE_URL": "demo.microstrategy.com/MicroStrategyLibrary",
  "MSTR_USERNAME": "administrator",
  "MSTR_PASSWORD": "yourPassword",
  "MSTR_PROJECT_ID": "B7CA92F04B9FAE8D941C3E9B7E0CD754",
  "MSTR_REPORT_ID": "FFDAB82F4CA397073ABD4196FCBCD918",
  "MSTR_CUBE_ID": "your_cube_id_here"
}
```

## API Functions

### Authentication

#### `login()`
Authenticates with the MicroStrategy REST API and returns session credentials.

```cpp
int login(const std::string& base_url, 
          const std::string& username, 
          const std::string& password,
          std::string& auth_token, 
          std::string& cookies);
```

**Endpoint:** `POST /api/auth/login`

**Parameters:**
- `base_url` - MicroStrategy server URL
- `username` - User account name
- `password` - User password
- `auth_token` - (output) X-MSTR-AuthToken for subsequent requests
- `cookies` - (output) Session cookies (JSESSIONID, etc.)

**Returns:** 0 on success, -1 on failure

**Output:** `login_response.json`

---

#### `logout()`
Terminates the authenticated session.

```cpp
int logout(const std::string& base_url, 
           const std::string& auth_token, 
           const std::string& cookies);
```

**Endpoint:** `POST /api/auth/logout`

**Parameters:**
- `base_url` - MicroStrategy server URL
- `auth_token` - Authentication token from login
- `cookies` - Session cookies from login

**Returns:** 0 on success

---

### Project Management

#### `get_projects()`
Retrieves list of projects the authenticated user can access.

```cpp
int get_projects(const std::string& base_url, 
                 const std::string& auth_token, 
                 const std::string& cookies);
```

**Endpoint:** `GET /api/projects`

**Parameters:**
- `base_url` - MicroStrategy server URL
- `auth_token` - Authentication token
- `cookies` - Session cookies

**Returns:** 0 on success, -1 on failure

**Output:** `projects.json`

---

### Report Operations

#### `get_report()`
Retrieves detailed report definition including metadata and expressions.

```cpp
int get_report(const std::string& base_url, 
               const std::string& auth_token,
               const std::string& cookies, 
               const std::string& project_id,
               const std::string& report_id);
```

**Endpoint:** `GET /api/model/reports/{reportId}?showExpressionAs=tree`

**Parameters:**
- `base_url` - MicroStrategy server URL
- `auth_token` - Authentication token
- `cookies` - Session cookies
- `project_id` - Target project ID
- `report_id` - Report ID to retrieve

**Returns:** 0 on success, -1 on failure

**Output:** `report_{reportId}.json`

---

### Cube Operations

#### `create_cube()`
Creates a cube instance for data retrieval and manipulation.

```cpp
int create_cube(const std::string& base_url, 
                const std::string& auth_token,
                const std::string& cookies, 
                const std::string& project_id,
                const std::string& cube_id);
```

**Endpoint:** `POST /api/cubes/{cubeId}/instances`

**Parameters:**
- `base_url` - MicroStrategy server URL
- `auth_token` - Authentication token
- `cookies` - Session cookies
- `project_id` - Target project ID
- `cube_id` - Cube ID to instantiate

**Returns:** 0 on success, -1 on failure

**Output:** `cube_instance_{cubeId}.json`

---

## Utility Functions

### `extract_value()`
Extracts JSON string values by key (simple parser).

```cpp
std::string extract_value(const std::string& content, 
                          const std::string& key);
```

### `extract_header_value()`
Extracts HTTP header values (case-insensitive).

```cpp
std::string extract_header_value(const std::vector<std::string>& headers, 
                                 const std::string& key);
```

---

## SSL/TLS Communication

### `ssl_read()`
Low-level function for HTTPS requests with SSL/TLS.

```cpp
int ssl_read(const std::string& host, 
             const std::string& port_num, 
             const std::string& http,
             std::string& response, 
             std::vector<std::string>& headers);
```

---

## Usage Example

```cpp
#include "get.hh"

int main() {
    std::string auth_token, cookies;
    if (login("demo.microstrategy.com/MicroStrategyLibrary", 
              "administrator", "password", 
              auth_token, cookies) != 0) {
        return -1;
    }
    
    get_projects("demo.microstrategy.com/MicroStrategyLibrary", 
                 auth_token, cookies);
    
    get_report("demo.microstrategy.com/MicroStrategyLibrary",
               auth_token, cookies,
               "B7CA92F04B9FAE8D941C3E9B7E0CD754",
               "FFDAB82F4CA397073ABD4196FCBCD918");
    
    logout("demo.microstrategy.com/MicroStrategyLibrary", 
           auth_token, cookies);
    
    return 0;
}
```

## Output Files

The client generates JSON output files for inspection:
- `login_response.json` - Authentication response
- `projects.json` - Available projects list
- `report_{id}.json` - Report definitions
- `cube_instance_{id}.json` - Cube instance details
- `exception.txt` - Error details (if exceptions occur)


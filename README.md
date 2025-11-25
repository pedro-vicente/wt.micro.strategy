# MicroStrategy Web Client

A C++ web application that provides an interactive interface for MicroStrategy business intelligence reports and financial data analysis. 

## Live demo

[https://pedro-vicente.net:9448](https://pedro-vicente.net:9448) 

![Demo](https://github.com/user-attachments/assets/efc78b31-f14a-4a5b-a80b-842f38cd486e)

## Features

- **MicroStrategy REST API Integration**: Full-featured client for authentication, project management, report retrieval, and cube operations
- **Web-Based UI**: Modern, responsive interface with navigation between Projects, Search, Library, Report, and Data views
- **SSL/TLS Security**: Native HTTPS support using Boost.ASIO with OpenSSL
- **Database Connectivity**: ODBC integration for financial data (FinMart) access
- **SQLite Support**: Embedded database for local data storage and caching
- **Cross-Platform**: Builds on Windows (MSVC), Linux (GCC), and macOS (Clang)

## Architecture

The application follows a modular design:

- **Web Server**: Wt framework handles HTTP/HTTPS serving and UI rendering
- **API Client**: Custom C++ client for MicroStrategy REST API communication
- **Database Layer**: ODBC and SQLite interfaces for data access
- **UI Components**: Widget-based interface with login, projects, search, library, report, and data views

## Technology Stack

- **Framework**: [Wt (Web Toolkit)](https://www.webtoolkit.eu/wt) 4.12.1
- **Networking**: [Boost.ASIO](https://www.boost.org/doc/libs/release/doc/html/boost_asio.html) 1.30.2 with OpenSSL
- **Build System**: CMake 3.28+
- **Database**: SQLite 3, ODBC
- **C++ Standard**: C++17
- **Dependencies**: Boost 1.88 (filesystem, thread, program_options, chrono)

## MicroStrategy Documentation

### Official API Resources

- **REST API Reference**: [https://microstrategy.github.io/rest-api-docs](https://microstrategy.github.io/rest-api-docs)
- **Authentication**: [https://microstrategy.github.io/rest-api-docs/#/Authentication](https://microstrategy.github.io/rest-api-docs/#/Authentication)
- **Projects**: [https://microstrategy.github.io/rest-api-docs/#/Projects](https://microstrategy.github.io/rest-api-docs/#/Projects)
- **Reports**: [https://microstrategy.github.io/rest-api-docs/#/Reports](https://microstrategy.github.io/rest-api-docs/#/Reports)
- **Cubes**: [https://microstrategy.github.io/rest-api-docs/#/Cubes](https://microstrategy.github.io/rest-api-docs/#/Cubes)
- **Library**: [https://microstrategy.github.io/rest-api-docs/#/Library](https://microstrategy.github.io/rest-api-docs/#/Library)

### Developer Resources

- **MicroStrategy Developer Zone**: [https://community.microstrategy.com/s/developers](https://community.microstrategy.com/s/developers)
- **REST API Samples**: [https://github.com/MicroStrategy/rest-api-samples](https://github.com/MicroStrategy/rest-api-samples)
- **SDK Documentation**: [https://www2.microstrategy.com/producthelp/current/RESTSDK/Content/topics/REST_API/REST_API.htm](https://www2.microstrategy.com/producthelp/current/RESTSDK/Content/topics/REST_API/REST_API.htm)

## Configuration

Create a `config.json` file in the working directory:

```json
{
  "MSTR_BASE_URL": "demo.microstrategy.com/MicroStrategyLibrary",
  "MSTR_USERNAME": "administrator",
  "MSTR_PASSWORD": "your_password",
  "MSTR_PROJECT_ID": "B7CA92F04B9FAE8D941C3E9B7E0CD754",
  "MSTR_REPORT_ID": "FFDAB82F4CA397073ABD4196FCBCD918",
  "MSTR_CUBE_ID": "your_cube_id_here"
}
```

## Building

### Prerequisites

- CMake 3.28 or higher
- C++17 compatible compiler (GCC 13, Clang, MSVC 2022)
- Boost 1.88 libraries
- OpenSSL 3.0+
- ODBC driver (unixODBC on Linux, built-in on Windows/macOS)

### Build Scripts

**Linux/macOS:**
```bash
./build.cmake.sh
```

**Windows:**
```batch
build.boost.bat
```

## Running

### Linux/macOS
```bash
./strategy --http-address=0.0.0.0 --http-port=8080 --docroot=.
```

### Windows
```cmd
strategy.exe --http-address=0.0.0.0 --http-port=8080 --docroot=.
```

Access the application at `http://localhost:8080`

## API Usage

See [API.md](API.md) for detailed documentation on the MicroStrategy REST API client functions, including authentication, project queries, report retrieval, and cube operations.

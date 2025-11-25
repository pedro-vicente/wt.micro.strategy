#ifndef LITE_HH
#define LITE_HH

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <ctime>
#include <cstring>
#include <sqlite3.h>
#include <iomanip>
#include <algorithm>

#include "finmart.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
// database manager
/////////////////////////////////////////////////////////////////////////////////////////////////////

class finmart_db
{
private:
  sqlite3* db;

public:
  finmart_db(const std::string& db_path);
  ~finmart_db();
  void initialize_database();
  void populate_sample_data();
  std::vector<transaction> get_all_transactions();
  std::map<std::string, double> get_department_spending();
  std::map<std::string, int> get_source_system_counts();
  double get_total_spending();
  bool is_open() const;
};

#endif

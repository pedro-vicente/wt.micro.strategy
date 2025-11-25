#include "odbc.hh"
#include <iostream>

const std::string server("localhost");
const std::string database("test_db");
odbc query;
void show(const table_t& table);
int test_null();
int test_float_1();
int test_float_2();

/////////////////////////////////////////////////////////////////////////////////////////////////////
//main
/////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
  std::string conn = make_conn(server, database);
  if (query.connect(conn) < 0) assert(0);

  if (test_null() < 0) assert(0);
  if (test_float_1() < 0) assert(0);
  if (test_float_2() < 0) assert(0);

  query.disconnect();
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//show
/////////////////////////////////////////////////////////////////////////////////////////////////////

void show(const table_t& table)
{
  for (size_t idx_row = 0; idx_row < table.rows.size(); idx_row++)
  {
    row_t r = table.rows.at(idx_row);
    for (size_t idx_col = 0; idx_col < table.cols.size(); idx_col++)
    {
      std::string s = r.col.at(idx_col);
      std::cout << s << " (" << s.size() << ") ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//test_null
/////////////////////////////////////////////////////////////////////////////////////////////////////

int test_null()
{
  table_t table;
  std::string sql;

  sql = "DROP TABLE IF EXISTS table_1;";
  if (query.exec_direct(sql) < 0) assert(0);
  sql = "CREATE TABLE table_1 ([Id] [int] NOT NULL, [Name] [char](36) NULL, [Adress] [varchar](100) NULL, [Time] [datetime] NULL);";
  if (query.exec_direct(sql) < 0) assert(0);

  sql = "INSERT INTO table_1 ([Id], [Name], [Adress], [Time]) VALUES (0, 'Bob', '123 Tree Rd.', '2022-02-22');";
  if (query.exec_direct(sql) < 0) assert(0);

  sql = "INSERT INTO table_1 ([Id], [Name], [Adress], [Time]) VALUES (1, 'Susan', '456 Bee Rd.', '2022-02-22');";
  if (query.exec_direct(sql) < 0) assert(0);

  sql = "INSERT INTO table_1 ([Id], [Name], [Adress], [Time]) VALUES (2, '', '', '');";
  if (query.exec_direct(sql) < 0) assert(0);

  sql = "INSERT INTO table_1 ([Id], [Name], [Adress], [Time]) VALUES (3, NULL, NULL, NULL);";
  if (query.exec_direct(sql) < 0) assert(0);

  if (query.fetch("SELECT * FROM [table_1] ORDER BY [Id];", table) < 0) assert(0);

  show(table);

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//test_float_1
/////////////////////////////////////////////////////////////////////////////////////////////////////

int test_float_1()
{
  table_t table;
  std::string sql;

  sql = "DROP TABLE IF EXISTS table_2;";
  if (query.exec_direct(sql) < 0) assert(0);

  sql = "CREATE table table_2 (a float, b varchar(10));";
  if (query.exec_direct(sql) < 0) assert(0);

  sql = "INSERT INTO table_2 VALUES (1.4, 'foo');";
  if (query.exec_direct(sql) < 0) assert(0);

  if (query.fetch("SELECT * FROM [table_2];", table) < 0) assert(0);

  show(table);

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//test_float_2
/////////////////////////////////////////////////////////////////////////////////////////////////////

int test_float_2()
{
  table_t table;
  std::string sql;

  sql = "DROP TABLE IF EXISTS table_3;";
  if (query.exec_direct(sql) < 0) assert(0);

  sql = "CREATE table table_3 (a float, b varchar(10));";
  if (query.exec_direct(sql) < 0) assert(0);

  sql = "INSERT INTO table_3 VALUES (1.4, 'foo');";
  if (query.exec_direct(sql) < 0) assert(0);

  //STR ( float_expression [ , length [ , decimal ] ] )  
  //float_expression Is an expression of approximate numeric(float) data type with a decimal point.
  //length Is the total length.This includes decimal point, sign, digits, and spaces.The default is 10.
  //decimal Is the number of places to the right of the decimal point.decimal must be less than or equal to 16. If decimal is more than 16 then the result is truncated to sixteen places to the right of the decimal point.

  if (query.fetch("SELECT str(a,3,1) FROM [table_3];", table) < 0) assert(0);

  show(table);

  return 0;
}


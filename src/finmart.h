#ifndef FINMART_H
#define FINMART_H

#include <string>

/////////////////////////////////////////////////////////////////////////////////////////////////////
// transaction
/////////////////////////////////////////////////////////////////////////////////////////////////////

struct transaction
{
  int id;
  std::string date;
  std::string department;
  std::string category;
  std::string vendor;
  double amount;
  std::string status;
  std::string source_system;  // PeopleSoft, Coupa, SAP, Legacy, MicroStrategy
};

#endif

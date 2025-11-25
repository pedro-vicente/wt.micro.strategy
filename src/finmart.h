#ifndef FINMART_H
#define FINMART_H

#include <string>

/////////////////////////////////////////////////////////////////////////////////////////////////////
// transaction - represents a financial transaction from FinMart
// Mirrors data from enterprise systems like PeopleSoft and Coupa
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

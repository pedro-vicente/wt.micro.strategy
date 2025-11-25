#include "data.hh"
#include "app.hh"
#include "db_interface.hh"
#include "odbc.hh"
#include <Wt/WBreak.h>
#include <sstream>
#include <iomanip>

#define USE_SQLSERVER 0

#if USE_SQLSERVER
const DatabaseBackend DB_BACKEND = DatabaseBackend::SQLSERVER;
const std::string DB_CONNECTION = make_conn("localhost", "finmart_db", "", "");
#else
const DatabaseBackend DB_BACKEND = DatabaseBackend::SQLITE;
const std::string DB_CONNECTION = "finmart.db";
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////
// WidgetView
// aggregates data from:
// - PeopleSoft (financial/HR)
// - Coupa (procurement)
// - SAP (enterprise)
// - MicroStrategy (reports/analytics)
/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetView::WidgetView(WApplicationStrategy* app)
  : app(app)
{
  setStyleClass("data-view");

  addWidget(std::make_unique<Wt::WText>("<h3>FinMart Data Warehouse</h3>"));

  Wt::WContainerWidget* toolbar = addWidget(std::make_unique<Wt::WContainerWidget>());
  toolbar->setStyleClass("toolbar");

  refresh_btn = toolbar->addWidget(std::make_unique<Wt::WPushButton>("Refresh Data"));
  refresh_btn->setStyleClass("btn btn-sm btn-outline-primary");
  refresh_btn->clicked().connect(this, &WidgetView::refresh);

  Wt::WPushButton* run_etl_btn = toolbar->addWidget(std::make_unique<Wt::WPushButton>("Run ETL"));
  run_etl_btn->setStyleClass("btn btn-sm btn-info");
  run_etl_btn->clicked().connect(this, &WidgetView::load_data);

  toolbar->addWidget(std::make_unique<Wt::WText>(" Filter: "));

  filter_combo = toolbar->addWidget(std::make_unique<Wt::WComboBox>());
  filter_combo->addItem("All Departments");
  filter_combo->addItem("IT");
  filter_combo->addItem("HR");
  filter_combo->addItem("Finance");
  filter_combo->addItem("Operations");
  filter_combo->addItem("Procurement");
  filter_combo->changed().connect(this, &WidgetView::apply_filter);

  status_text = toolbar->addWidget(std::make_unique<Wt::WText>());
  status_text->setStyleClass("status-text");

  Wt::WContainerWidget* summary_box = addWidget(std::make_unique<Wt::WContainerWidget>());
  summary_box->setStyleClass("summary-box");
  total_spending_text = summary_box->addWidget(std::make_unique<Wt::WText>());

  addWidget(std::make_unique<Wt::WBreak>());

  Wt::WContainerWidget* summary_row = addWidget(std::make_unique<Wt::WContainerWidget>());
  summary_row->setStyleClass("summary-row");

  Wt::WContainerWidget* dept_box = summary_row->addWidget(std::make_unique<Wt::WContainerWidget>());
  dept_box->setStyleClass("summary-panel");
  dept_box->addWidget(std::make_unique<Wt::WText>("<h4>Department Spending</h4>"));
  summary_table = dept_box->addWidget(std::make_unique<Wt::WTable>());
  summary_table->setStyleClass("table table-striped");
  summary_table->setHeaderCount(1);

  addWidget(std::make_unique<Wt::WBreak>());

  addWidget(std::make_unique<Wt::WText>("<h4>Recent Transactions</h4>"));
  table = addWidget(std::make_unique<Wt::WTable>());
  table->setStyleClass("table table-striped table-hover data-table");
  table->setHeaderCount(1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// refresh
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetView::refresh()
{
  load_data();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// load_data
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetView::load_data()
{
  IFinMartDatabase* db = DatabaseFactory::create(DB_BACKEND, DB_CONNECTION);
  
  if (!db)
  {
    status_text->setText("Error: Could not create database instance");
    return;
  }

  std::string backend_name = db->get_backend_name();
  status_text->setText("Loading data from " + backend_name + "...");

  try
  {
    if (!db->is_open())
    {
      status_text->setText("Error: Database not connected");
      delete db;
      return;
    }

    double total = db->get_total_spending();
    std::stringstream ss;
    ss << "<div style='font-size:1.5em;padding:15px;background:#e3f2fd;border-radius:5px;margin:10px 0;'>"
      << "<b>Total Spending:</b> $" << std::fixed << std::setprecision(2) << total
      << " <span style='font-size:0.6em;color:#666;'>(" << backend_name << ")</span>"
      << "</div>";
    total_spending_text->setText(ss.str());

    show_department_summary(db);
    std::vector<transaction> transactions = db->get_all_transactions();
    table->clear();

    table->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("<b>ID</b>"));
    table->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("<b>Date</b>"));
    table->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("<b>Department</b>"));
    table->elementAt(0, 3)->addWidget(std::make_unique<Wt::WText>("<b>Category</b>"));
    table->elementAt(0, 4)->addWidget(std::make_unique<Wt::WText>("<b>Vendor</b>"));
    table->elementAt(0, 5)->addWidget(std::make_unique<Wt::WText>("<b>Amount</b>"));
    table->elementAt(0, 6)->addWidget(std::make_unique<Wt::WText>("<b>Status</b>"));
    table->elementAt(0, 7)->addWidget(std::make_unique<Wt::WText>("<b>Source</b>"));

    std::string filter = filter_combo->currentText().toUTF8();

    int row = 1;
    int count = 0;
    for (size_t idx = 0; idx < transactions.size(); idx++)
    {
      const transaction& t = transactions[idx];
      if (filter != "All Departments" && t.department != filter)
        continue;

      table->elementAt(row, 0)->addWidget(
        std::make_unique<Wt::WText>(std::to_string(t.id)));
      table->elementAt(row, 1)->addWidget(
        std::make_unique<Wt::WText>(t.date));
      table->elementAt(row, 2)->addWidget(
        std::make_unique<Wt::WText>(t.department));
      table->elementAt(row, 3)->addWidget(
        std::make_unique<Wt::WText>(t.category));
      table->elementAt(row, 4)->addWidget(
        std::make_unique<Wt::WText>(t.vendor));

      std::stringstream amt_str;
      amt_str << "$" << std::fixed << std::setprecision(2) << t.amount;
      table->elementAt(row, 5)->addWidget(
        std::make_unique<Wt::WText>(amt_str.str()));

      std::string status_style = (t.status == "Pending")
        ? "color:orange;font-weight:bold;"
        : "color:green;";
      table->elementAt(row, 6)->addWidget(
        std::make_unique<Wt::WText>("<span style='" + status_style + "'>" + t.status + "</span>"));

      std::string source_icon;
      if (t.source_system == "PeopleSoft") source_icon = "PS ";
      else if (t.source_system == "Coupa") source_icon = "CO ";
      else if (t.source_system == "SAP") source_icon = "SAP ";
      else if (t.source_system == "MicroStrategy") source_icon = "MS ";
      else source_icon = "LG ";

      table->elementAt(row, 7)->addWidget(
        std::make_unique<Wt::WText>(source_icon + t.source_system));

      row++;
      count++;
    }

    status_text->setText("Loaded " + std::to_string(count) + " transactions from " + backend_name);
    app->set_status("FinMart data refreshed - " + std::to_string(count) + " records (" + backend_name + ")");
    
    delete db;
  }
  catch (const std::exception& e)
  {
    status_text->setText("Error loading data: " + std::string(e.what()));
    app->set_status("Database error", true);
    delete db;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// show_department_summary
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetView::show_department_summary(IFinMartDatabase* db)
{
  std::map<std::string, double> spending = db->get_department_spending();

  summary_table->clear();

  summary_table->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("<b>Department</b>"));
  summary_table->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("<b>Total Spending</b>"));

  int row = 1;
  for (std::map<std::string, double>::const_iterator it = spending.begin(); it != spending.end(); ++it)
  {
    const std::string& dept = it->first;
    const double amount = it->second;

    summary_table->elementAt(row, 0)->addWidget(
      std::make_unique<Wt::WText>(dept));

    std::stringstream amt_str;
    amt_str << "$" << std::fixed << std::setprecision(2) << amount;
    summary_table->elementAt(row, 1)->addWidget(
      std::make_unique<Wt::WText>(amt_str.str()));

    row++;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// show_source_system_counts
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetView::show_source_system_counts(IFinMartDatabase* db)
{
  std::map<std::string, int> counts = db->get_source_system_counts();

  //add chart

  for (std::map<std::string, int>::const_iterator it = counts.begin(); it != counts.end(); ++it)
  {
    const std::string& system = it->first;
    const int count = it->second;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// apply_filter
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetView::apply_filter()
{
  load_data();
}

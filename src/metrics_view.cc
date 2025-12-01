#include "metrics_view.hh"
#include "app.hh"
#include "db_interface.hh"
#include "metrics.hh"
#include <Wt/WBreak.h>
#include <sstream>
#include <iomanip>

#define USE_SQLSERVER 0

#if USE_SQLSERVER
const DatabaseBackend METRICS_DB_BACKEND = DatabaseBackend::SQLSERVER;
const std::string METRICS_DB_CONNECTION = "";
#else
const DatabaseBackend METRICS_DB_BACKEND = DatabaseBackend::SQLITE;
const std::string METRICS_DB_CONNECTION = "finmart.db";
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////
// WidgetMetrics
/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetMetrics::WidgetMetrics(WApplicationStrategy* app)
  : app(app)
{
  setStyleClass("data-view");

  addWidget(std::make_unique<Wt::WText>("<h3>Financial Metrics Dashboard</h3>"));

  Wt::WContainerWidget* toolbar = addWidget(std::make_unique<Wt::WContainerWidget>());
  toolbar->setStyleClass("toolbar");

  refresh_btn = toolbar->addWidget(std::make_unique<Wt::WPushButton>("Refresh"));
  refresh_btn->setStyleClass("btn btn-sm btn-outline-primary");
  refresh_btn->clicked().connect(this, &WidgetMetrics::refresh);

  toolbar->addWidget(std::make_unique<Wt::WText>(" Company: "));

  company_combo = toolbar->addWidget(std::make_unique<Wt::WComboBox>());
  company_combo->addItem("All Companies");
  company_combo->addItem("ACME Corp");
  company_combo->addItem("TechVentures");
  company_combo->addItem("GlobalTrade");
  company_combo->changed().connect(this, &WidgetMetrics::on_company_changed);

  status_text = toolbar->addWidget(std::make_unique<Wt::WText>());
  status_text->setStyleClass("status-text");

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // calculated metrics section
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  Wt::WContainerWidget* metrics_box = addWidget(std::make_unique<Wt::WContainerWidget>());
  metrics_box->setStyleClass("summary-panel");
  metrics_box->addWidget(std::make_unique<Wt::WText>("<h4>Calculated Financial Metrics</h4>"));
  metrics_table = metrics_box->addWidget(std::make_unique<Wt::WTable>());
  metrics_table->setStyleClass("table table-striped");
  metrics_table->setHeaderCount(1);

  addWidget(std::make_unique<Wt::WBreak>());

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // raw records section
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  addWidget(std::make_unique<Wt::WText>("<h4>Financial Records (Raw Data)</h4>"));
  records_table = addWidget(std::make_unique<Wt::WTable>());
  records_table->setStyleClass("table table-striped table-hover data-table");
  records_table->setHeaderCount(1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// refresh
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetMetrics::refresh()
{
  load_metrics();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// on_company_changed
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetMetrics::on_company_changed()
{
  load_metrics();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// load_metrics
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetMetrics::load_metrics()
{
  IFinMartDatabase* db = DatabaseFactory::create(METRICS_DB_BACKEND, METRICS_DB_CONNECTION);

  if (!db)
  {
    status_text->setText("Error: Could not create database instance");
    return;
  }

  std::string backend_name = db->get_backend_name();
  status_text->setText("Loading metrics from " + backend_name + "...");

  try
  {
    if (!db->is_open())
    {
      status_text->setText("Error: Database not connected");
      delete db;
      return;
    }

    show_financial_records(db);
    show_calculated_metrics(db);

    app->set_status("Financial metrics loaded from " + backend_name);
    delete db;
  }
  catch (const std::exception& e)
  {
    status_text->setText("Error: " + std::string(e.what()));
    app->set_status("Database error", true);
    delete db;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// show_financial_records
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetMetrics::show_financial_records(IFinMartDatabase* db)
{
  std::string filter = company_combo->currentText().toUTF8();
  std::vector<FinancialRecord> records;

  if (filter == "All Companies")
  {
    records = db->get_financial_records();
  }
  else
  {
    records = db->get_financial_records_by_company(filter);
  }

  records_table->clear();

  records_table->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("<b>Period</b>"));
  records_table->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("<b>Company</b>"));
  records_table->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("<b>Revenue</b>"));
  records_table->elementAt(0, 3)->addWidget(std::make_unique<Wt::WText>("<b>COGS</b>"));
  records_table->elementAt(0, 4)->addWidget(std::make_unique<Wt::WText>("<b>Op. Expenses</b>"));
  records_table->elementAt(0, 5)->addWidget(std::make_unique<Wt::WText>("<b>Total Assets</b>"));
  records_table->elementAt(0, 6)->addWidget(std::make_unique<Wt::WText>("<b>Total Liabilities</b>"));

  int row = 1;
  for (size_t idx = 0; idx < records.size(); ++idx)
  {
    const FinancialRecord& r = records[idx];

    records_table->elementAt(row, 0)->addWidget(
      std::make_unique<Wt::WText>(r.period));
    records_table->elementAt(row, 1)->addWidget(
      std::make_unique<Wt::WText>(r.company_id));

    std::stringstream rev_str;
    rev_str << "$" << std::fixed << std::setprecision(0) << r.revenue;
    records_table->elementAt(row, 2)->addWidget(
      std::make_unique<Wt::WText>(rev_str.str()));

    std::stringstream cogs_str;
    cogs_str << "$" << std::fixed << std::setprecision(0) << r.cogs;
    records_table->elementAt(row, 3)->addWidget(
      std::make_unique<Wt::WText>(cogs_str.str()));

    std::stringstream opex_str;
    opex_str << "$" << std::fixed << std::setprecision(0) << r.operating_expenses;
    records_table->elementAt(row, 4)->addWidget(
      std::make_unique<Wt::WText>(opex_str.str()));

    std::stringstream assets_str;
    assets_str << "$" << std::fixed << std::setprecision(0) << r.total_assets;
    records_table->elementAt(row, 5)->addWidget(
      std::make_unique<Wt::WText>(assets_str.str()));

    std::stringstream liab_str;
    liab_str << "$" << std::fixed << std::setprecision(0) << r.total_liabilities;
    records_table->elementAt(row, 6)->addWidget(
      std::make_unique<Wt::WText>(liab_str.str()));

    ++row;
  }

  status_text->setText("Loaded " + std::to_string(records.size()) + " records");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// show_calculated_metrics
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetMetrics::show_calculated_metrics(IFinMartDatabase* db)
{
  std::string filter = company_combo->currentText().toUTF8();
  std::vector<FinancialRecord> records;

  if (filter == "All Companies")
  {
    records = db->get_financial_records();
  }
  else
  {
    records = db->get_financial_records_by_company(filter);
  }

  metrics_table->clear();

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // header row
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  metrics_table->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("<b>Period</b>"));
  metrics_table->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("<b>Company</b>"));
  metrics_table->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("<b>Gross Profit</b>"));
  metrics_table->elementAt(0, 3)->addWidget(std::make_unique<Wt::WText>("<b>Gross Margin</b>"));
  metrics_table->elementAt(0, 4)->addWidget(std::make_unique<Wt::WText>("<b>EBITDA</b>"));
  metrics_table->elementAt(0, 5)->addWidget(std::make_unique<Wt::WText>("<b>Net Income</b>"));
  metrics_table->elementAt(0, 6)->addWidget(std::make_unique<Wt::WText>("<b>Net Margin</b>"));
  metrics_table->elementAt(0, 7)->addWidget(std::make_unique<Wt::WText>("<b>Current Ratio</b>"));
  metrics_table->elementAt(0, 8)->addWidget(std::make_unique<Wt::WText>("<b>D/E Ratio</b>"));
  metrics_table->elementAt(0, 9)->addWidget(std::make_unique<Wt::WText>("<b>ROA</b>"));
  metrics_table->elementAt(0, 10)->addWidget(std::make_unique<Wt::WText>("<b>ROE</b>"));

  int row = 1;
  for (size_t idx = 0; idx < records.size(); ++idx)
  {
    const FinancialRecord& r = records[idx];

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // convert to FinancialMetrics to use calculation methods
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    FinancialMetrics m;
    m.period = r.period;
    m.company_id = r.company_id;
    m.revenue = r.revenue;
    m.cogs = r.cogs;
    m.operating_expenses = r.operating_expenses;
    m.depreciation = r.depreciation;
    m.amortization = r.amortization;
    m.interest = r.interest;
    m.taxes = r.taxes;
    m.current_assets = r.current_assets;
    m.current_liabilities = r.current_liabilities;
    m.inventory = r.inventory;
    m.total_assets = r.total_assets;
    m.total_liabilities = r.total_liabilities;

    metrics_table->elementAt(row, 0)->addWidget(
      std::make_unique<Wt::WText>(m.period));
    metrics_table->elementAt(row, 1)->addWidget(
      std::make_unique<Wt::WText>(m.company_id));

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // gross profit
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    std::stringstream gp_str;
    gp_str << "$" << std::fixed << std::setprecision(0) << m.gross_profit();
    metrics_table->elementAt(row, 2)->addWidget(
      std::make_unique<Wt::WText>(gp_str.str()));

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // gross margin (percentage)
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    std::stringstream gm_str;
    gm_str << std::fixed << std::setprecision(1) << (m.gross_margin() * 100) << "%";
    metrics_table->elementAt(row, 3)->addWidget(
      std::make_unique<Wt::WText>(gm_str.str()));

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // EBITDA
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    std::stringstream ebitda_str;
    ebitda_str << "$" << std::fixed << std::setprecision(0) << m.ebitda();
    metrics_table->elementAt(row, 4)->addWidget(
      std::make_unique<Wt::WText>(ebitda_str.str()));

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // net income
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    std::stringstream ni_str;
    ni_str << "$" << std::fixed << std::setprecision(0) << m.net_income();
    std::string ni_style = (m.net_income() >= 0) ? "color:green;" : "color:red;";
    metrics_table->elementAt(row, 5)->addWidget(
      std::make_unique<Wt::WText>("<span style='" + ni_style + "'>" + ni_str.str() + "</span>"));

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // net margin (percentage)
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    std::stringstream nm_str;
    nm_str << std::fixed << std::setprecision(1) << (m.net_margin() * 100) << "%";
    std::string nm_style = (m.net_margin() >= 0) ? "color:green;" : "color:red;";
    metrics_table->elementAt(row, 6)->addWidget(
      std::make_unique<Wt::WText>("<span style='" + nm_style + "'>" + nm_str.str() + "</span>"));

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // current ratio
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    std::stringstream cr_str;
    cr_str << std::fixed << std::setprecision(2) << m.current_ratio();
    std::string cr_style = (m.current_ratio() >= 1.5) ? "color:green;" : 
                           (m.current_ratio() >= 1.0) ? "color:orange;" : "color:red;";
    metrics_table->elementAt(row, 7)->addWidget(
      std::make_unique<Wt::WText>("<span style='" + cr_style + "'>" + cr_str.str() + "</span>"));

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // debt to equity ratio
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    std::stringstream de_str;
    de_str << std::fixed << std::setprecision(2) << m.debt_to_equity();
    std::string de_style = (m.debt_to_equity() <= 1.0) ? "color:green;" : 
                           (m.debt_to_equity() <= 2.0) ? "color:orange;" : "color:red;";
    metrics_table->elementAt(row, 8)->addWidget(
      std::make_unique<Wt::WText>("<span style='" + de_style + "'>" + de_str.str() + "</span>"));

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // return on assets (percentage)
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    std::stringstream roa_str;
    roa_str << std::fixed << std::setprecision(1) << (m.return_on_assets() * 100) << "%";
    std::string roa_style = (m.return_on_assets() >= 0.05) ? "color:green;" : 
                            (m.return_on_assets() >= 0) ? "color:orange;" : "color:red;";
    metrics_table->elementAt(row, 9)->addWidget(
      std::make_unique<Wt::WText>("<span style='" + roa_style + "'>" + roa_str.str() + "</span>"));

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // return on equity (percentage)
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    std::stringstream roe_str;
    roe_str << std::fixed << std::setprecision(1) << (m.return_on_equity() * 100) << "%";
    std::string roe_style = (m.return_on_equity() >= 0.15) ? "color:green;" : 
                            (m.return_on_equity() >= 0.10) ? "color:orange;" : "color:red;";
    metrics_table->elementAt(row, 10)->addWidget(
      std::make_unique<Wt::WText>("<span style='" + roe_style + "'>" + roe_str.str() + "</span>"));

    ++row;
  }
}

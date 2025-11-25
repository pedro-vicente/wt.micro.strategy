#include "search.hh"
#include "app.hh"
#include "api.hh"
#include <Wt/WBreak.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////
// WidgetSearch
/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetSearch::WidgetSearch(WApplicationStrategy* app)
  : app(app)
{
  setStyleClass("search-view");

  addWidget(std::make_unique<Wt::WText>("<h3>Search</h3>"));

  Wt::WContainerWidget* form = addWidget(std::make_unique<Wt::WContainerWidget>());
  form->setStyleClass("search-form");

  form->addWidget(std::make_unique<Wt::WText>("Search:"));
  search_input = form->addWidget(std::make_unique<Wt::WLineEdit>());
  search_input->setPlaceholderText("Enter search term...");
  search_input->setWidth(200);

  form->addWidget(std::make_unique<Wt::WText>("Type:"));
  type_combo = form->addWidget(std::make_unique<Wt::WComboBox>());
  type_combo->addItem("All");
  type_combo->addItem("Reports (3)");
  type_combo->addItem("Dashboards (55)");
  type_combo->addItem("Cubes (21)");

  Wt::WPushButton* search_btn = form->addWidget(std::make_unique<Wt::WPushButton>("Search"));
  search_btn->setStyleClass("btn btn-primary");
  search_btn->clicked().connect(this, &WidgetSearch::do_search);

  status_text = form->addWidget(std::make_unique<Wt::WText>());
  status_text->setStyleClass("status-text");

  addWidget(std::make_unique<Wt::WBreak>());

  table = addWidget(std::make_unique<Wt::WTable>());
  table->setStyleClass("table table-striped table-hover");
  table->setHeaderCount(1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// refresh
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetSearch::refresh()
{
  do_search();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// do_search
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetSearch::do_search()
{
  if (!app->session().authenticated || app->session().projectId.empty())
  {
    status_text->setText("Please select a project first");
    return;
  }

  std::string query = search_input->text().toUTF8();
  std::string type_str = type_combo->currentText().toUTF8();

  int type = 0;
  if (type_str.find("3") != std::string::npos) type = 3;      // Reports
  else if (type_str.find("55") != std::string::npos) type = 55; // Dashboards
  else if (type_str.find("21") != std::string::npos) type = 21; // Cubes

  status_text->setText("Searching...");

  std::string response;
  search(app->session(), query, type, 50, response);

  table->clear();

  table->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("<b>Name</b>"));
  table->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("<b>Type</b>"));
  table->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("<b>ID</b>"));
  table->elementAt(0, 3)->addWidget(std::make_unique<Wt::WText>("<b>Action</b>"));

  std::vector<SearchResult> results = parse_search_results(response);

  int row = 1;
  for (size_t idx = 0; idx < results.size(); idx++)
  {
    const SearchResult& item = results[idx];
    table->elementAt(row, 0)->addWidget(std::make_unique<Wt::WText>(item.name));

    std::string type_name = "Unknown";
    if (item.type == "3") type_name = "Report";
    else if (item.type == "55") type_name = "Dashboard";
    else if (item.type == "21") type_name = "Cube";
    table->elementAt(row, 1)->addWidget(std::make_unique<Wt::WText>(type_name));

    Wt::WText* id_text = table->elementAt(row, 2)->addWidget(
      std::make_unique<Wt::WText>(item.id));
    id_text->setStyleClass("monospace small");

    Wt::WPushButton* view_btn = table->elementAt(row, 3)->addWidget(
      std::make_unique<Wt::WPushButton>("View"));
    view_btn->setStyleClass("btn btn-sm btn-outline-primary");

    std::string item_id = item.id;
    std::string item_name = item.name;
    std::string item_type = item.type;
    view_btn->clicked().connect([this, item_id, item_name, item_type]() {
      view_item(item_id, item_name, item_type);
      });

    row++;
  }

  status_text->setText("Found " + std::to_string(results.size()) + " results");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// view_item
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetSearch::view_item(const std::string& id, const std::string& name, const std::string& type)
{
  if (type == "3")  // report
  {
    app->show_report(id, name);
  }
  else
  {
    app->set_status("Viewing " + name + " (type " + type + ")");
  }
}

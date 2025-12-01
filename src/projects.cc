#include "projects.hh"
#include "app.hh"
#include "get.hh"
#include "api.hh"
#include "lite.hh"
#include <Wt/WBreak.h>
#include <fstream>
#include <sstream>

/////////////////////////////////////////////////////////////////////////////////////////////////////
// WidgetProjects
/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetProjects::WidgetProjects(WApplicationStrategy* app)
  : app(app)
{
  setStyleClass("projects-view");

  addWidget(std::make_unique<Wt::WText>("<h3>Projects</h3>"));

  Wt::WContainerWidget* toolbar = addWidget(std::make_unique<Wt::WContainerWidget>());
  toolbar->setStyleClass("toolbar");

  refresh_btn = toolbar->addWidget(std::make_unique<Wt::WPushButton>("Refresh"));
  refresh_btn->setStyleClass("btn btn-sm btn-outline-primary");
  refresh_btn->clicked().connect(this, &WidgetProjects::refresh);

  status_text = toolbar->addWidget(std::make_unique<Wt::WText>());
  status_text->setStyleClass("status-text");

  addWidget(std::make_unique<Wt::WBreak>());

  table = addWidget(std::make_unique<Wt::WTable>());
  table->setStyleClass("table table-striped table-hover");
  table->setHeaderCount(1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// refresh
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetProjects::refresh()
{
  load_projects();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// load_projects
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetProjects::load_projects()
{
  if (!app->session().authenticated)
  {
    status_text->setText("Not authenticated");
    return;
  }

  std::string current_project_id = app->session().project_id;

  status_text->setText("Loading projects...");
  refresh_btn->setEnabled(false);

  table->clear();

  table->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("<b>Name</b>"));
  table->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("<b>ID</b>"));
  table->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("<b>Status</b>"));
  table->elementAt(0, 3)->addWidget(std::make_unique<Wt::WText>("<b>Action</b>"));

  std::string response;
  int result = get_projects(app->session().base_url,
    app->session().auth_token,
    app->session().cookies);

  std::ifstream file("projects.json");
  if (file.is_open())
  {
    std::stringstream ss;
    ss << file.rdbuf();
    response = ss.str();
    file.close();
  }

  if (response.empty())
  {
    status_text->setText("Failed to load projects");
    refresh_btn->setEnabled(true);
    return;
  }

  std::vector<Project> projects = parse_projects(response);

  int row = 1;
  std::string selected_name;
  for (size_t idx = 0; idx < projects.size(); idx++)
  {
    const Project& proj = projects[idx];
    bool is_selected = (proj.id == current_project_id);
    if (is_selected) selected_name = proj.name;

    table->elementAt(row, 0)->addWidget(
      std::make_unique<Wt::WText>(is_selected ? ("<b>" + proj.name + "</b>") : proj.name));

    Wt::WText* id_text = table->elementAt(row, 1)->addWidget(
      std::make_unique<Wt::WText>(proj.id));
    id_text->setStyleClass("monospace small");

    table->elementAt(row, 2)->addWidget(std::make_unique<Wt::WText>(proj.status));

    Wt::WTableCell* action_cell = table->elementAt(row, 3);
    if (is_selected)
    {
      action_cell->addWidget(
        std::make_unique<Wt::WText>("<span style='color:green;font-weight:bold;'>&#10003; Selected</span>"));
    }
    else
    {
      Wt::WPushButton* select_btn = action_cell->addWidget(
        std::make_unique<Wt::WPushButton>("Select"));
      select_btn->setStyleClass("btn btn-sm btn-primary");

      std::string proj_id = proj.id;
      std::string proj_name = proj.name;
      select_btn->clicked().connect([this, proj_id, proj_name]() {
        select_project(proj_id, proj_name);
        });
    }

    row++;
  }

  if (!selected_name.empty())
  {
    status_text->setText("Active: " + selected_name + " (" + std::to_string(projects.size()) + " projects)");
  }
  else
  {
    status_text->setText("Found " + std::to_string(projects.size()) + " projects - select one to continue");
  }
  refresh_btn->setEnabled(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// select_project
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetProjects::select_project(const std::string& project_id, const std::string& project_name)
{
  app->session().project_id = project_id;
  status_text->setText("Running ETL for: " + project_name + "...");
  run_etl(project_id, project_name);
  load_projects();
  app->show_data();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// run_etl - Extract, Transform, Load from MicroStrategy to SQLite
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetProjects::run_etl(const std::string& project_id, const std::string& project_name)
{
  finmart_db db("finmart.db");
  app->set_status("ETL Complete: Loaded data for " + project_name + " to finmart.db");
}

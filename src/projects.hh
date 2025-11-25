#ifndef PROJECTS_HH
#define PROJECTS_HH

#include <Wt/WContainerWidget.h>
#include <Wt/WTable.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

class WApplicationStrategy;

class WidgetProjects : public Wt::WContainerWidget
{
public:
  WidgetProjects(WApplicationStrategy* app);

  void refresh();

private:
  void load_projects();
  void select_project(const std::string& project_id, const std::string& project_name);
  void run_etl(const std::string& project_id, const std::string& project_name);

  WApplicationStrategy* app;
  Wt::WTable* table;
  Wt::WText* status_text;
  Wt::WPushButton* refresh_btn;
};

#endif

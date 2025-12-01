#ifndef APP_HH
#define APP_HH

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WNavigationBar.h>
#include <Wt/WMenu.h>
#include <Wt/WText.h>
#include <memory>
#include "api.hh"

class WidgetLogin;
class WidgetProjects;
class WidgetLibrary;
class WidgetView;
class WidgetMetrics;

/////////////////////////////////////////////////////////////////////////////////////////////////////
// WApplicationStrategy 
/////////////////////////////////////////////////////////////////////////////////////////////////////

class WApplicationStrategy : public Wt::WApplication
{
public:
  WApplicationStrategy(const Wt::WEnvironment& env);
  Session& session() { return m_session; }
  const Session& session() const { return m_session; }
  void show_login();
  void show_main();
  void show_projects();
  void show_library();
  void show_data();
  void show_metrics();
  void set_status(const std::string& message, bool is_error = false);

private:
  void create_login_page();
  void create_main_page();
  void on_login();
  void on_logout();
  Session m_session;
  Wt::WContainerWidget* wt_root;
  Wt::WStackedWidget* pages;
  Wt::WContainerWidget* login_page;
  Wt::WContainerWidget* main_page;
  Wt::WNavigationBar* navbar;
  Wt::WMenu* menu;
  Wt::WText* status_text;

  WidgetLogin* p_login;
  WidgetProjects* p_projects;
  WidgetLibrary* p_library;
  WidgetView* p_data;
  WidgetMetrics* p_metrics;
};

#endif

#include "app.hh"
#include "login.hh"
#include "projects.hh"
#include "library.hh"
#include "data.hh"
#include "metrics_view.hh"
#include "get.hh"

#include <Wt/WBreak.h>
#include <Wt/WPushButton.h>
#include <Wt/WMenuItem.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////
// WApplicationStrategy 
/////////////////////////////////////////////////////////////////////////////////////////////////////

WApplicationStrategy::WApplicationStrategy(const Wt::WEnvironment& env)
  : WApplication(env)
{
  setTitle("FinMart");

  Wt::WCssStyleSheet& css = styleSheet();

  css.addRule("body",
    "font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;"
    "margin: 0; padding: 0; background: #f5f5f5;");
  css.addRule(".container-fluid", "max-width: 1400px; margin: 0 auto;");

  css.addRule(".login-view",
    "max-width: 600px; margin: 50px auto; padding: 30px; background: #fff;"
    "border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1);");
  css.addRule(".login-title", "color: #333; margin-bottom: 10px;");
  css.addRule(".login-subtitle", "color: #666; margin-bottom: 20px;");
  css.addRule(".login-form", "margin: 20px 0;");
  css.addRule(".login-form label", "display: inline-block; width: 120px; margin: 5px 0;");
  css.addRule(".login-buttons", "margin-top: 20px;");
  css.addRule(".login-buttons button", "margin-right: 10px;");
  css.addRule(".login-error", "color: #dc3545; margin-top: 10px; display: block;");
  css.addRule(".login-info", "margin-top: 30px; padding: 15px; background: #f0f0f0; border-radius: 5px;");
  css.addRule(".login-info h4", "margin-top: 0; color: #495057;");

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // main container and navbar
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  css.addRule(".main-container", "padding: 20px; background: #fff; min-height: 100vh;");
  css.addRule(".navbar",
    "background: #f8f9fa; padding: 10px 20px; border-bottom: 1px solid #dee2e6;"
    "display: flex; align-items: center; flex-wrap: nowrap;");
  css.addRule(".navbar-nav", "display: flex !important; list-style: none; margin: 0; padding: 0; flex-wrap: nowrap;");
  css.addRule(".navbar-nav li", "margin-right: 5px; display: inline-block !important;");
  css.addRule(".navbar-nav a, .navbar-nav .nav-link",
    "padding: 8px 15px; text-decoration: none; color: #555; border-radius: 4px;");
  css.addRule(".navbar-nav a:hover, .navbar-nav .nav-link:hover", "background: #e9ecef;");
  css.addRule(".navbar-nav .active a, .navbar-nav .active .nav-link", 
    "background: #0d6efd; color: #fff; font-weight: bold; box-shadow: 0 2px 4px rgba(0,0,0,0.2);");
  css.addRule(".navbar-right", "margin-left: auto; display: flex; align-items: center;");
  css.addRule(".navbar-collapse", "display: block !important; visibility: visible !important;");

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // buttons
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  css.addRule(".btn",
    "padding: 8px 16px; border: 1px solid #ccc; border-radius: 4px;"
    "cursor: pointer; font-size: 14px; background: #fff;");
  css.addRule(".btn:hover", "background: #f0f0f0;");
  css.addRule(".btn-primary", "background: #007bff; color: #fff; border-color: #007bff;");
  css.addRule(".btn-primary:hover", "background: #0056b3;");
  css.addRule(".btn-secondary", "background: #6c757d; color: #fff; border-color: #6c757d;");
  css.addRule(".btn-secondary:hover", "background: #545b62;");
  css.addRule(".btn-info", "background: #17a2b8; color: #fff; border-color: #17a2b8;");
  css.addRule(".btn-info:hover", "background: #138496;");
  css.addRule(".btn-outline-danger", "background: transparent; color: #dc3545; border-color: #dc3545;");
  css.addRule(".btn-outline-danger:hover", "background: #dc3545; color: #fff;");
  css.addRule(".btn-outline-primary", "background: transparent; color: #007bff; border-color: #007bff;");
  css.addRule(".btn-outline-primary:hover", "background: #007bff; color: #fff;");
  css.addRule(".btn-outline-secondary", "background: transparent; color: #6c757d; border-color: #6c757d;");
  css.addRule(".btn-outline-secondary:hover", "background: #6c757d; color: #fff;");
  css.addRule(".btn-outline-info", "background: transparent; color: #17a2b8; border-color: #17a2b8;");
  css.addRule(".btn-outline-info:hover", "background: #17a2b8; color: #fff;");
  css.addRule(".btn-sm", "padding: 4px 10px; font-size: 12px;");

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // toolbar and status
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  css.addRule(".toolbar", "margin: 10px 0; padding: 10px 0;");
  css.addRule(".toolbar button", "margin-right: 10px;");
  css.addRule(".status-text", "color: #6c757d; margin-left: 15px;");

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // tables
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  css.addRule(".table", "width: 100%; border-collapse: collapse; margin-top: 15px;");
  css.addRule(".table th, .table td", "padding: 10px 12px; text-align: left; border-bottom: 1px solid #dee2e6;");
  css.addRule(".table th", "background: #f8f9fa; font-weight: 600;");
  css.addRule(".table tr:hover, .table-hover tr:hover", "background: #f5f5f5;");
  css.addRule(".table-striped tr:nth-child(even)", "background: #f9f9f9;");
  css.addRule(".monospace", "font-family: monospace;");
  css.addRule(".small", "font-size: 0.85em;");

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // search form
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  css.addRule(".search-form",
    "margin: 15px 0; padding: 15px; background: #f8f9fa; border-radius: 5px;"
    "display: flex; align-items: center; flex-wrap: wrap;");
  css.addRule(".search-form label", "margin: 0 5px 0 15px;");
  css.addRule(".search-form input, .search-form select",
    "margin-right: 10px; padding: 6px 10px; border: 1px solid #ccc; border-radius: 4px;");

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // report view
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  css.addRule(".report-header", "margin-bottom: 20px; padding-bottom: 15px; border-bottom: 1px solid #dee2e6;");
  css.addRule(".report-title", "margin-left: 20px; display: inline-block;");
  css.addRule(".report-title h3", "display: inline; margin-right: 10px;");
  css.addRule(".report-title small", "color: #6c757d;");
  css.addRule(".data-table", "font-size: 0.9em;");
  css.addRule(".data-table td", "max-width: 300px; overflow: hidden; text-overflow: ellipsis; white-space: nowrap;");

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // JSON container
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  css.addRule(".json-container", "margin-top: 20px; padding: 15px; background: #f8f9fa; border-radius: 5px;");
  css.addRule(".json-area", "width: 100%; font-family: monospace; font-size: 0.85em; border: 1px solid #ccc; padding: 10px;");

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // view padding
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  css.addRule(".projects-view, .search-view, .library-view, .report-view, .data-view", "padding: 20px;");

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // metrics view
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  css.addRule(".metrics-panel", "margin: 15px 0; padding: 15px; background: #fff; border: 1px solid #dee2e6; border-radius: 5px;");
  css.addRule(".metrics-panel h4", "margin-top: 0; color: #1565c0; border-bottom: 1px solid #e0e0e0; padding-bottom: 10px;");

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // data view
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  css.addRule(".data-view h3", "color: #1565c0; border-bottom: 2px solid #1565c0; padding-bottom: 10px;");
  css.addRule(".summary-box", "margin: 15px 0;");
  css.addRule(".summary-row", "display: flex; gap: 20px; margin: 20px 0;");
  css.addRule(".summary-panel",
    "flex: 1; padding: 15px; background: #fff; border: 1px solid #dee2e6; border-radius: 5px;");
  css.addRule(".summary-panel h4", "margin-top: 0; color: #495057;");

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // status bar
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  css.addRule(".status-bar", "margin-top: 20px; padding: 10px; background: #f8f9fa; border-top: 1px solid #dee2e6;");
  css.addRule(".text-muted", "color: #6c757d;");
  css.addRule(".text-danger", "color: #dc3545;");

  wt_root = root();
  wt_root->setStyleClass("container-fluid");
  pages = wt_root->addWidget(std::make_unique<Wt::WStackedWidget>());
  create_login_page();
  create_main_page();
  show_login();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// create_login_page 
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WApplicationStrategy::create_login_page()
{
  login_page = pages->addWidget(std::make_unique<Wt::WContainerWidget>());
  p_login = login_page->addWidget(std::make_unique<WidgetLogin>(this));
  p_login->login_success().connect(this, &WApplicationStrategy::on_login);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// create_main_page 
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WApplicationStrategy::create_main_page()
{
  main_page = pages->addWidget(std::make_unique<Wt::WContainerWidget>());
  main_page->setStyleClass("main-container");
  navbar = main_page->addWidget(std::make_unique<Wt::WNavigationBar>());
  navbar->setStyleClass("navbar");
  navbar->setResponsive(false);
  Wt::WStackedWidget* content_stack = main_page->addWidget(std::make_unique<Wt::WStackedWidget>());
  content_stack->setStyleClass("content-area");
  menu = navbar->addMenu(std::make_unique<Wt::WMenu>(content_stack));
  menu->setStyleClass("navbar-nav");

  p_projects = new WidgetProjects(this);
  std::unique_ptr<Wt::WMenuItem> projects_item = std::make_unique<Wt::WMenuItem>("Projects", std::unique_ptr<WidgetProjects>(p_projects));
  menu->addItem(std::move(projects_item));

  p_library = new WidgetLibrary(this);
  std::unique_ptr<Wt::WMenuItem> library_item = std::make_unique<Wt::WMenuItem>("Library", std::unique_ptr<WidgetLibrary>(p_library));
  menu->addItem(std::move(library_item));

  p_data = new WidgetView(this);
  std::unique_ptr<Wt::WMenuItem> data_item = std::make_unique<Wt::WMenuItem>("Data (FinMart)", std::unique_ptr<WidgetView>(p_data));
  menu->addItem(std::move(data_item));

  p_metrics = new WidgetMetrics(this);
  std::unique_ptr<Wt::WMenuItem> metrics_item = std::make_unique<Wt::WMenuItem>("Metrics", std::unique_ptr<WidgetMetrics>(p_metrics));
  Wt::WMenuItem* metrics_menuitem = menu->addItem(std::move(metrics_item));
  metrics_menuitem->triggered().connect([this]() {
    p_metrics->load_metrics();
  });

  Wt::WMenu* right_menu = navbar->addMenu(std::make_unique<Wt::WMenu>(), Wt::AlignmentFlag::Right);
  right_menu->setStyleClass("navbar-nav navbar-right");

  std::unique_ptr<Wt::WPushButton> logout_btn_ptr = std::make_unique<Wt::WPushButton>("Logout");
  Wt::WPushButton* logout_btn = logout_btn_ptr.get();
  logout_btn->setStyleClass("btn btn-outline-danger btn-sm");
  logout_btn->clicked().connect(this, &WApplicationStrategy::on_logout);
  navbar->addWidget(std::move(logout_btn_ptr), Wt::AlignmentFlag::Right);

  Wt::WContainerWidget* status_bar = main_page->addWidget(std::make_unique<Wt::WContainerWidget>());
  status_bar->setStyleClass("status-bar");
  status_text = status_bar->addWidget(std::make_unique<Wt::WText>());
  status_text->setStyleClass("text-muted small");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// on_login 
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WApplicationStrategy::on_login()
{
  show_main();
  set_status("Connected to " + m_session.base_url);
  p_projects->refresh();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// on_logout 
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WApplicationStrategy::on_logout()
{
  logout(m_session.base_url, m_session.auth_token, m_session.cookies);
  m_session = Session();
  show_login();
  set_status("Logged out");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// show_login 
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WApplicationStrategy::show_login()
{
  pages->setCurrentWidget(login_page);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// show_main 
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WApplicationStrategy::show_main()
{
  pages->setCurrentWidget(main_page);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// show_projects 
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WApplicationStrategy::show_projects()
{
  menu->select(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// show_library 
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WApplicationStrategy::show_library()
{
  menu->select(1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// set_status 
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WApplicationStrategy::set_status(const std::string& message, bool is_error)
{
  if (status_text)
  {
    status_text->setText(message);
    if (is_error)
    {
      status_text->setStyleClass("text-danger small");
    }
    else
    {
      status_text->setStyleClass("text-muted small");
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// show_data 
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WApplicationStrategy::show_data()
{
  menu->select(2);
  p_data->load_data();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// show_metrics 
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WApplicationStrategy::show_metrics()
{
  menu->select(3);
  p_metrics->load_metrics();
}

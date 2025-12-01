#include "login.hh"
#include "app.hh"
#include "get.hh"
#include <Wt/WBreak.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////
// WidgetLogin
/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetLogin::WidgetLogin(WApplicationStrategy* app)
  : app(app)
{
  setStyleClass("login-view");

  addWidget(std::make_unique<Wt::WText>("<h2 class='login-title'>MicroStrategy</h2>"));

  Wt::WContainerWidget* form = addWidget(std::make_unique<Wt::WContainerWidget>());
  form->setStyleClass("login-form");

  form->addWidget(std::make_unique<Wt::WText>("Server URL:"));
  server_url = form->addWidget(std::make_unique<Wt::WLineEdit>());
  server_url->setPlaceholderText("https://demo.microstrategy.com/MicroStrategyLibrary");
  server_url->setWidth(350);
  form->addWidget(std::make_unique<Wt::WBreak>());

  form->addWidget(std::make_unique<Wt::WText>("Username:"));
  username = form->addWidget(std::make_unique<Wt::WLineEdit>());
  username->setPlaceholderText("Enter username (or use Guest Login)");
  username->setWidth(350);
  form->addWidget(std::make_unique<Wt::WBreak>());

  form->addWidget(std::make_unique<Wt::WText>("Password:"));
  password = form->addWidget(std::make_unique<Wt::WLineEdit>());
  password->setEchoMode(Wt::EchoMode::Password);
  password->setPlaceholderText("Enter password");
  password->setWidth(350);
  form->addWidget(std::make_unique<Wt::WBreak>());

  Wt::WContainerWidget* buttons = addWidget(std::make_unique<Wt::WContainerWidget>());
  buttons->setStyleClass("login-buttons");

  Wt::WPushButton* login_btn = buttons->addWidget(std::make_unique<Wt::WPushButton>("Login"));
  login_btn->setStyleClass("btn btn-primary");
  login_btn->clicked().connect(this, &WidgetLogin::do_login);

  Wt::WPushButton* guest_btn = buttons->addWidget(std::make_unique<Wt::WPushButton>("Guest Login"));
  guest_btn->setStyleClass("btn btn-secondary");
  guest_btn->clicked().connect(this, &WidgetLogin::do_guest_login);

  Wt::WPushButton* demo_btn = buttons->addWidget(std::make_unique<Wt::WPushButton>("Use Demo Server"));
  demo_btn->setStyleClass("btn btn-info");
  demo_btn->clicked().connect(this, &WidgetLogin::use_demo_server);

  error_text = addWidget(std::make_unique<Wt::WText>());
  error_text->setStyleClass("login-error");

  use_demo_server();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// use_demo_server
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetLogin::use_demo_server()
{
  server_url->setText("https://demo.microstrategy.com/MicroStrategyLibrary");
  error_text->setText("");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// do_login
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetLogin::do_login()
{
  std::string url = server_url->text().toUTF8();
  std::string user = username->text().toUTF8();
  std::string pass = password->text().toUTF8();

  if (url.empty())
  {
    error_text->setText("Please enter a server URL");
    return;
  }

  error_text->setText("Connecting...");

  while (!url.empty() && url.back() == '/')
  {
    url.pop_back();
  }

  std::string auth_token;
  std::string cookies;

  int result = login(url, user, pass, auth_token, cookies);

  if (result == 0 && !auth_token.empty())
  {
    app->session().base_url = url;
    app->session().auth_token = auth_token;
    app->session().cookies = cookies;
    app->session().username = user.empty() ? "Guest" : user;
    app->session().authenticated = true;

    error_text->setText("");
    m_login_success.emit();
  }
  else
  {
    error_text->setText("Login failed. Check credentials and server URL.");
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// do_guest_login
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetLogin::do_guest_login()
{
  std::string url = server_url->text().toUTF8();

  if (url.empty())
  {
    error_text->setText("Please enter a server URL");
    return;
  }

  error_text->setText("Connecting as guest...");

  while (!url.empty() && url.back() == '/')
  {
    url.pop_back();
  }

  std::string auth_token;
  std::string cookies;

  int result = login(url, "", "", auth_token, cookies);

  if (result == 0 && !auth_token.empty())
  {
    app->session().base_url = url;
    app->session().auth_token = auth_token;
    app->session().cookies = cookies;
    app->session().username = "Guest";
    app->session().authenticated = true;

    error_text->setText("");
    m_login_success.emit();
  }
  else
  {
    error_text->setText("Guest login failed. Server may not support guest access.");
  }
}

#ifndef LOGIN_HH
#define LOGIN_HH

#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WSignal.h>

class WApplicationStrategy;

/////////////////////////////////////////////////////////////////////////////////////////////////////
// WidgetLibrary
/////////////////////////////////////////////////////////////////////////////////////////////////////


class WidgetLogin : public Wt::WContainerWidget
{
public:
  WidgetLogin(WApplicationStrategy* app);

  Wt::Signal<>& login_success() { return m_login_success; }

private:
  void do_login();
  void do_guest_login();
  void use_demo_server();

  WApplicationStrategy* app;
  Wt::WLineEdit* server_url;
  Wt::WLineEdit* username;
  Wt::WLineEdit* password;
  Wt::WText* error_text;
  Wt::Signal<> m_login_success;
};

#endif

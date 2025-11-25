#ifndef REPORT_VIEW_HH
#define REPORT_VIEW_HH

#include <Wt/WContainerWidget.h>
#include <Wt/WTable.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WTextArea.h>

class WApplicationStrategy;

class WidgetReport : public Wt::WContainerWidget
{
public:
  WidgetReport(WApplicationStrategy* app);
  void load_report(const std::string& report_id, const std::string& report_name);

private:
  void execute_report();
  void toggle_json();

  WApplicationStrategy* app;
  std::string report_id;
  std::string report_name;
  Wt::WText* title_text;
  Wt::WText* status_text;
  Wt::WTable* data_table;
  Wt::WTextArea* json_area;
  Wt::WContainerWidget* json_container;
  Wt::WPushButton* json_btn;
  bool json_visible;
  std::string raw_response;
};

#endif

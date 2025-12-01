#ifndef METRICS_VIEW_HH
#define METRICS_VIEW_HH

#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WTable.h>
#include <Wt/WPushButton.h>
#include <Wt/WComboBox.h>

class WApplicationStrategy;
class IFinMartDatabase;

/////////////////////////////////////////////////////////////////////////////////////////////////////
// WidgetMetrics
/////////////////////////////////////////////////////////////////////////////////////////////////////

class WidgetMetrics : public Wt::WContainerWidget
{
public:
  WidgetMetrics(WApplicationStrategy* app);
  void refresh();
  void load_metrics();

private:
  WApplicationStrategy* app;
  Wt::WText* status_text;
  Wt::WPushButton* refresh_btn;
  Wt::WTable* records_table;
  Wt::WTable* metrics_table;
  Wt::WComboBox* company_combo;

  void show_financial_records(IFinMartDatabase* db);
  void show_calculated_metrics(IFinMartDatabase* db);
  void on_company_changed();
};

#endif

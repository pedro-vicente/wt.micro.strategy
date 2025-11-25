#ifndef DATA_HH
#define DATA_HH

#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WTable.h>
#include <Wt/WPushButton.h>
#include <Wt/WComboBox.h>

class WApplicationStrategy;
class IFinMartDatabase;

/////////////////////////////////////////////////////////////////////////////////////////////////////
// WidgetView
/////////////////////////////////////////////////////////////////////////////////////////////////////

class WidgetView : public Wt::WContainerWidget
{
public:
  WidgetView(WApplicationStrategy* app);
  void refresh();
  void load_data();

private:
  WApplicationStrategy* app;
  Wt::WText* status_text;
  Wt::WPushButton* refresh_btn;
  Wt::WTable* table;
  Wt::WTable* summary_table;
  Wt::WText* total_spending_text;
  Wt::WComboBox* filter_combo;

  void show_department_summary(IFinMartDatabase* db);
  void show_source_system_counts(IFinMartDatabase* db);
  void apply_filter();
};

#endif

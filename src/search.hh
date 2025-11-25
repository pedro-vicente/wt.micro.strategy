#ifndef SEARCH_HH
#define SEARCH_HH

#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WComboBox.h>
#include <Wt/WTable.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

class WApplicationStrategy;

class WidgetSearch : public Wt::WContainerWidget
{
public:
  WidgetSearch(WApplicationStrategy* app);
  void refresh();

private:
  void do_search();
  void view_item(const std::string& id, const std::string& name, const std::string& type);

  WApplicationStrategy* app;
  Wt::WLineEdit* search_input;
  Wt::WComboBox* type_combo;
  Wt::WTable* table;
  Wt::WText* status_text;
};

#endif

#ifndef LIBRARY_HH
#define LIBRARY_HH

#include <Wt/WContainerWidget.h>
#include <Wt/WTable.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

class WApplicationStrategy;

/////////////////////////////////////////////////////////////////////////////////////////////////////
// WidgetLibrary
/////////////////////////////////////////////////////////////////////////////////////////////////////

class WidgetLibrary : public Wt::WContainerWidget
{
public:
  WidgetLibrary(WApplicationStrategy* app);
  void refresh();

private:
  void load_library();

  WApplicationStrategy* app;
  Wt::WTable* table;
  Wt::WText* status_text;
  Wt::WPushButton* refresh_btn;
};

#endif

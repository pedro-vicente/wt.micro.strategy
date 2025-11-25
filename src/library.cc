#include "library.hh"
#include "app.hh"
#include "api.hh"
#include <Wt/WBreak.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////
// WidgetLibrary
/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetLibrary::WidgetLibrary(WApplicationStrategy* app)
  : app(app)
{
  setStyleClass("library-view");

  addWidget(std::make_unique<Wt::WText>("<h3>Library</h3>"));

  Wt::WContainerWidget* toolbar = addWidget(std::make_unique<Wt::WContainerWidget>());
  toolbar->setStyleClass("toolbar");

  refresh_btn = toolbar->addWidget(std::make_unique<Wt::WPushButton>("Refresh"));
  refresh_btn->setStyleClass("btn btn-sm btn-outline-primary");
  refresh_btn->clicked().connect(this, &WidgetLibrary::refresh);

  status_text = toolbar->addWidget(std::make_unique<Wt::WText>());
  status_text->setStyleClass("status-text");

  addWidget(std::make_unique<Wt::WBreak>());

  table = addWidget(std::make_unique<Wt::WTable>());
  table->setStyleClass("table table-striped table-hover");
  table->setHeaderCount(1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// refresh
/////////////////////////////////////////////////////////////////////////////////////////////////////


void WidgetLibrary::refresh()
{
  load_library();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// load_library
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetLibrary::load_library()
{
  if (!app->session().authenticated)
  {
    status_text->setText("Not authenticated");
    return;
  }

  status_text->setText("Loading library...");
  refresh_btn->setEnabled(false);

  std::string response;
  get_library(app->session(), 50, response);

  // Clear and rebuild table
  table->clear();

  table->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("<b>Name</b>"));
  table->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("<b>Type</b>"));
  table->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("<b>Modified</b>"));
  table->elementAt(0, 3)->addWidget(std::make_unique<Wt::WText>("<b>ID</b>"));

  std::vector<LibraryItem> items = parse_library_items(response);

  int row = 1;
  for (size_t idx = 0; idx < items.size(); idx++)
  {
    const LibraryItem& item = items[idx];
    table->elementAt(row, 0)->addWidget(std::make_unique<Wt::WText>(item.name));
    table->elementAt(row, 1)->addWidget(std::make_unique<Wt::WText>(item.type));
    table->elementAt(row, 2)->addWidget(std::make_unique<Wt::WText>(item.dateModified));

    Wt::WText* id_text = table->elementAt(row, 3)->addWidget(
      std::make_unique<Wt::WText>(item.id));
    id_text->setStyleClass("monospace small");

    row++;
  }

  status_text->setText("Found " + std::to_string(items.size()) + " items");
  refresh_btn->setEnabled(true);
}

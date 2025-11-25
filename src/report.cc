#include "report.hh"
#include "app.hh"
#include "api.hh"
#include <Wt/WBreak.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////
// WidgetReport
/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetReport::WidgetReport(WApplicationStrategy* app)
  : app(app), json_visible(false)
{
  setStyleClass("report-view");

  Wt::WContainerWidget* header = addWidget(std::make_unique<Wt::WContainerWidget>());
  header->setStyleClass("report-header");

  Wt::WPushButton* back_btn = header->addWidget(std::make_unique<Wt::WPushButton>("< Back"));
  back_btn->setStyleClass("btn btn-sm btn-outline-secondary");
  back_btn->clicked().connect([this]() { this->app->show_search(); });

  Wt::WContainerWidget* title_box = header->addWidget(std::make_unique<Wt::WContainerWidget>());
  title_box->setStyleClass("report-title");
  title_text = title_box->addWidget(std::make_unique<Wt::WText>());

  Wt::WContainerWidget* toolbar = addWidget(std::make_unique<Wt::WContainerWidget>());
  toolbar->setStyleClass("toolbar");

  Wt::WPushButton* refresh_btn = toolbar->addWidget(std::make_unique<Wt::WPushButton>("Refresh"));
  refresh_btn->setStyleClass("btn btn-sm btn-outline-primary");
  refresh_btn->clicked().connect(this, &WidgetReport::execute_report);

  json_btn = toolbar->addWidget(std::make_unique<Wt::WPushButton>("Show Raw JSON"));
  json_btn->setStyleClass("btn btn-sm btn-outline-info");
  json_btn->clicked().connect(this, &WidgetReport::toggle_json);

  status_text = toolbar->addWidget(std::make_unique<Wt::WText>());
  status_text->setStyleClass("status-text");

  addWidget(std::make_unique<Wt::WBreak>());

  data_table = addWidget(std::make_unique<Wt::WTable>());
  data_table->setStyleClass("table table-striped table-hover data-table");
  data_table->setHeaderCount(1);

  json_container = addWidget(std::make_unique<Wt::WContainerWidget>());
  json_container->setStyleClass("json-container");
  json_container->hide();

  json_container->addWidget(std::make_unique<Wt::WText>("<b>Raw JSON Response:</b>"));
  json_container->addWidget(std::make_unique<Wt::WBreak>());
  json_area = json_container->addWidget(std::make_unique<Wt::WTextArea>());
  json_area->setStyleClass("json-area");
  json_area->setRows(20);
  json_area->setReadOnly(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// load_report
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetReport::load_report(const std::string& id, const std::string& name)
{
  this->report_id = id;
  this->report_name = name;

  title_text->setText("<h3>" + name + "</h3><small>" + id + "</small>");

  execute_report();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// execute_report
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetReport::execute_report()
{
  if (report_id.empty())
  {
    status_text->setText("No report selected");
    return;
  }

  if (!app->session().authenticated || app->session().projectId.empty())
  {
    status_text->setText("Please select a project first");
    return;
  }

  status_text->setText("Executing report...");

  get_report(app->session(), report_id, raw_response);

  json_area->setText(raw_response);

  data_table->clear();

  std::vector<std::string> headers;
  size_t pos = raw_response.find("\"headers\"");
  if (pos != std::string::npos)
  {
    size_t name_pos = pos;
    while ((name_pos = raw_response.find("\"name\"", name_pos)) != std::string::npos)
    {
      size_t start = raw_response.find(":", name_pos) + 1;
      while (start < raw_response.length() && raw_response[start] == ' ') start++;
      if (raw_response[start] == '"')
      {
        start++;
        size_t end = raw_response.find("\"", start);
        if (end != std::string::npos)
        {
          headers.push_back(raw_response.substr(start, end - start));
        }
      }
      name_pos = start;
      if (headers.size() > 20) break;  // Limit columns
    }
  }

  if (!headers.empty())
  {
    for (size_t idx = 0; idx < headers.size(); idx++)
    {
      data_table->elementAt(0, idx)->addWidget(
        std::make_unique<Wt::WText>("<b>" + headers[idx] + "</b>"));
    }
  }
  else
  {
    data_table->elementAt(0, 0)->addWidget(
      std::make_unique<Wt::WText>("<b>Response</b>"));
    data_table->elementAt(1, 0)->addWidget(
      std::make_unique<Wt::WText>("Report executed - see Raw JSON for details"));
  }

  pos = raw_response.find("\"data\"");
  if (pos != std::string::npos)
  {

    int row = 1;
    size_t row_pos = pos;
    while ((row_pos = raw_response.find("[", row_pos + 1)) != std::string::npos && row < 50)
    {

      size_t end_bracket = raw_response.find("]", row_pos);
      if (end_bracket != std::string::npos && end_bracket - row_pos < 1000)
      {
        std::string row_data = raw_response.substr(row_pos + 1, end_bracket - row_pos - 1);

        int col = 0;
        size_t val_pos = 0;
        while (val_pos < row_data.length() && col < (int)headers.size())
        {
          while (val_pos < row_data.length() && (row_data[val_pos] == ' ' || row_data[val_pos] == ','))
            val_pos++;

          if (val_pos >= row_data.length()) break;

          std::string value;
          if (row_data[val_pos] == '"')
          {
            val_pos++;
            size_t end = row_data.find("\"", val_pos);
            if (end != std::string::npos)
            {
              value = row_data.substr(val_pos, end - val_pos);
              val_pos = end + 1;
            }
          }
          else
          {
            size_t end = row_data.find_first_of(",]", val_pos);
            if (end != std::string::npos)
            {
              value = row_data.substr(val_pos, end - val_pos);
              val_pos = end;
            }
          }

          if (!value.empty())
          {
            data_table->elementAt(row, col)->addWidget(
              std::make_unique<Wt::WText>(value));
            col++;
          }
        }

        if (col > 0) row++;
      }

      row_pos = end_bracket;
    }
  }

  status_text->setText("Report loaded");
  app->set_status("Executed report: " + report_name);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// toggle_json
/////////////////////////////////////////////////////////////////////////////////////////////////////

void WidgetReport::toggle_json()
{
  json_visible = !json_visible;

  if (json_visible)
  {
    json_container->show();
    json_btn->setText("Hide Raw JSON");
  }
  else
  {
    json_container->hide();
    json_btn->setText("Show Raw JSON");
  }
}

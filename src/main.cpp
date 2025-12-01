#include <Wt/WApplication.h>
#include <Wt/WServer.h>
#include "app.hh"

/////////////////////////////////////////////////////////////////////////////////////////////////////
// create_application
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Wt::WApplication> create_application(const Wt::WEnvironment& env)
{
  return std::make_unique<WApplicationStrategy>(env);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// main
/////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  try
  {
    Wt::WServer server(argc, argv, WTHTTP_CONFIGURATION);

    server.addEntryPoint(Wt::EntryPointType::Application, create_application);

    server.run();
  }
  catch (Wt::WServer::Exception& e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}

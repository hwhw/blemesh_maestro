#include "org-bluez-mesh-network1-client-glue.h"
#include "org-bluez-mesh-application1-server-glue.h"
#include <sdbus-c++/sdbus-c++.h>
#include <vector>
#include <string>
#include <iostream>
#include <unistd.h>

class ManagerAdaptor : public sdbus::AdaptorInterfaces< sdbus::ObjectManager_adaptor > {
public:
    ManagerAdaptor(sdbus::IConnection& connection, std::string path)
    : AdaptorInterfaces(connection, std::move(path))
    {
        registerAdaptor();
    }

    ~ManagerAdaptor()
    {
        unregisterAdaptor();
    }
};

class ApplicationAdaptor final : public sdbus::AdaptorInterfaces< org::bluez::mesh::Application1_adaptor, sdbus::ManagedObject_adaptor, sdbus::Properties_adaptor > {
  public:
    ApplicationAdaptor(sdbus::IConnection& connection, std::string path) : AdaptorInterfaces(connection, std::move(path)) {
      registerAdaptor();
    }
    ~ApplicationAdaptor() {
      unregisterAdaptor();
    }

    void JoinComplete(const uint64_t& token) override {
      std::cerr << "Join complete, token is " << token << std::endl;
    }

    void JoinFailed(const std::string& reason) override {
      std::cerr << "Join failed, reason: " << reason << std::endl;
    }

    uint16_t CompanyID() override {
      return 0x05f1;
    }
    uint16_t ProductID() override {
      return 1;
    }
    uint16_t VersionID() override {
      return 1;
    }
    uint16_t CRPL() override {
      return 0;
    }
};

class NetworkProxy final : public sdbus::ProxyInterfaces< org::bluez::mesh::Network1_proxy > {
  public:
    NetworkProxy(sdbus::IConnection& connection, std::string destination, std::string path)
    : ProxyInterfaces(connection, std::move(destination), std::move(path))
    {
        registerProxy();
    }

    ~NetworkProxy()
    {
        unregisterProxy();
    }
};


sdbus::IObject* g_service{};

void onAttached(sdbus::Signal& signal)
{
    std::string concatenatedString;
    signal >> concatenatedString;

    std::cerr << "Attached successfully" << std::endl;
}

int main(int argc, char *argv[])
{
    auto bus = sdbus::createSystemBusConnection();

    const char* serviceObjectPath = "/de/hilses/blemesh_maestro";
    auto service = sdbus::createObject(*bus, serviceObjectPath);

    g_service = service.get();
    const char* serviceInterfaceName = "de.hilses.Blemesh_maestro";

    service->finishRegistration();

    bus->enterEventLoopAsync();

    auto manager = std::make_unique<ManagerAdaptor>(*bus, "/de/hilses/blemesh_maestro");

    auto app = std::make_unique<ApplicationAdaptor>(*bus, "/de/hilses/blemesh_maestro/application");

    const char* destinationName = "org.bluez.mesh";
    const char* objectPath = "/org/bluez/mesh";
    NetworkProxy networkProxy(*bus, destinationName, objectPath);

    std::uint64_t myKey = 17130243692309702836ULL;

    {
        try {
          auto reply = networkProxy.Attach(serviceObjectPath, myKey);
        } catch(const sdbus::Error& e) {
          std::cerr << "Error: " << e.getName() << ", message: " << e.getMessage() << std::endl;
        }
    }

    while(true) {
      sleep(1);
    }
    
}

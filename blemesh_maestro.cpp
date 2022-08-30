#include "org-bluez-mesh-network1-client-glue.h"
#include "org-bluez-mesh-node1-client-glue.h"
#include "org-bluez-mesh-application1-server-glue.h"
#include "org-bluez-mesh-provisionagent1-server-glue.h"
#include "org-bluez-mesh-element1-server-glue.h"
#include <sdbus-c++/sdbus-c++.h>

#include <vector>
#include <string>
#include <iostream>
#include <unistd.h>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
namespace uuids = boost::uuids;

#include <boost/asio.hpp>
namespace asio = boost::asio;


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
    ApplicationAdaptor(sdbus::IConnection& connection, std::string path) : AdaptorInterfaces(connection, std::move(path)) , m_connection(connection) {
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
      return 0x40;
    }
  private:
    sdbus::IConnection& m_connection;
};

class ProvisionAgentAdaptor final : public sdbus::AdaptorInterfaces< org::bluez::mesh::ProvisionAgent1_adaptor, sdbus::ManagedObject_adaptor, sdbus::Properties_adaptor > {
  public:
    ProvisionAgentAdaptor(sdbus::IConnection& connection, std::string path) : AdaptorInterfaces(connection, std::move(path)) {
      m_capabilities = {
        "out-numeric",
        //"public-oob"
      };
      m_oobinfo = {
        "other"
      };
      registerAdaptor();
    }
    ~ProvisionAgentAdaptor() {
      unregisterAdaptor();
    }
    std::vector<uint8_t> PrivateKey() override {
      // TODO: understand this better and change this, probably, as it is well-known example data
      // the according public key is ce9027b5375fe5d3ed3ac89cef6a8370f699a2d3130db02b87e7a632f15b0002e5b72c775127dc0ce686002ecbe057e3d6a8000d4fbf2cdfffe0d38a1c55a043
      std::cerr << "ProvisionAgent: sending private key" << std::endl;
      return {0x68, 0x72, 0xb1, 0x09, 0xea, 0x05, 0x74, 0xad, 0xcf, 0x88, 0xbf, 0x6d, 0xa6, 0x49, 0x96, 0xa4, 0x62, 0x4f, 0xe0, 0x18, 0x19, 0x1d, 0x93, 0x22, 0xa4, 0x95, 0x88, 0x37, 0x34, 0x12, 0x84, 0xbc};
    }
    std::vector<uint8_t> PublicKey() override {
      std::cerr << "ProvisionAgent: PublicKey() not implemented" << std::endl;
      return {};
    }
    void DisplayString(const std::string& value) override {
      std::cerr << "ProvisionAgent: DisplayString(" << value << ")" << std::endl;
    }
    void DisplayNumeric(const std::string& type, const uint32_t& number) override {
      std::cerr << "ProvisionAgent: DisplayNumeric(" << type << ", " << number << ")" << std::endl;
    }
    uint32_t PromptNumeric(const std::string& type) override {
      std::cerr << "ProvisionAgent: PromptNumeric(" << type << ")" << std::endl;
      std::cerr << "ProvisionAgent: not implemented." << std::endl;
      return {};
    }
    std::vector<uint8_t> PromptStatic(const std::string& type) override {
      std::cerr << "ProvisionAgent: PromptStatic(" << type << ")" << std::endl;
      std::cerr << "ProvisionAgent: not implemented." << std::endl;
      return {};
    }
    void Cancel() override {
      std::cerr << "ProvisionAgent: Cancel()" << std::endl;
    }
    std::vector<std::string> Capabilities() override {
      return m_capabilities;
    }
    std::vector<std::string> OutOfBandInfo() override {
      return m_oobinfo;
    }
  private:
    std::vector<std::string> m_capabilities;
    std::vector<std::string> m_oobinfo;
};

class ElementAdaptor final : public sdbus::AdaptorInterfaces< org::bluez::mesh::Element1_adaptor, sdbus::ManagedObject_adaptor, sdbus::Properties_adaptor > {
  public:
    ElementAdaptor(sdbus::IConnection& connection, std::string path, uint8_t index,
        std::vector<sdbus::Struct<uint16_t, std::map<std::string, sdbus::Variant>>> models,
        std::vector<sdbus::Struct<uint16_t, uint16_t, std::map<std::string, sdbus::Variant>>> vendormodels) : AdaptorInterfaces(connection, std::move(path))
    , m_index(index)
    , m_models(std::move(models))
    , m_vendormodels(std::move(vendormodels)) {
      registerAdaptor();
    }
    ~ElementAdaptor() {
      unregisterAdaptor();
    }
    void MessageReceived(const uint16_t& source, const uint16_t& key_index, const sdbus::Variant& destination, const std::vector<uint8_t>& data) override {
      std::cerr << "Element: MessageReceived" << std::endl;
    }
    void DevKeyMessageReceived(const uint16_t& source, const bool& remote, const uint16_t& net_index, const std::vector<uint8_t>& data) override {
      std::cerr << "Element: DevKeyMessageReceived" << std::endl;
    }
    void UpdateModelConfiguration(const uint16_t& model_id, const std::map<std::string, sdbus::Variant>& config) override {
      std::cerr << "Element: UpdateModelConfiguration" << std::endl;
    }
    uint8_t Index() override {
      return m_index;
    }
    std::vector<sdbus::Struct<uint16_t, std::map<std::string, sdbus::Variant>>> Models() override {
      return m_models;
    }
    std::vector<sdbus::Struct<uint16_t, uint16_t, std::map<std::string, sdbus::Variant>>> VendorModels() override {
      return m_vendormodels;
    }
  private:
    uint8_t m_index;
    std::vector<sdbus::Struct<uint16_t, std::map<std::string, sdbus::Variant>>> m_models;
    std::vector<sdbus::Struct<uint16_t, uint16_t, std::map<std::string, sdbus::Variant>>> m_vendormodels;
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

class NodeProxy final : public sdbus::ProxyInterfaces< org::bluez::mesh::Node1_proxy > {
  public:
    NodeProxy(sdbus::IConnection& connection, std::string destination, std::string path)
    : ProxyInterfaces(connection, std::move(destination), std::move(path))
    {
        registerProxy();
    }

    ~NodeProxy()
    {
        unregisterProxy();
    }
};

const char* obj_manager = "/de/hilses/blemesh_maestro";
const char* obj_app = "/de/hilses/blemesh_maestro/application";
const char* obj_agent = "/de/hilses/blemesh_maestro/agent";
const char* obj_ele00 = "/de/hilses/blemesh_maestro/application/ele00";

int main(int argc, char *argv[])
{
    po::options_description desc{"Options"};
    desc.add_options()
      ("join,j", po::value<uuids::uuid>(), "join a new node")
      ("leave,l", po::value<uint64_t>(), "node identified by given token leaves the network")
      ("attach,a", po::value<uint64_t>(), "node identified by given token attaches to the network and will listen on socket")
      ("socket,s", po::value<std::string>()->default_value("/tmp/blemesh_maestro"), "name of datagram UNIX socket that will listen to messages for sending");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    auto bus = sdbus::createSystemBusConnection();

    bus->enterEventLoopAsync();

    auto manager = std::make_unique<ManagerAdaptor>(*bus, obj_manager);
    auto app = std::make_unique<ApplicationAdaptor>(*bus, obj_app);
    auto provisionagent = std::make_unique<ProvisionAgentAdaptor>(*bus, obj_agent);

    std::vector<sdbus::Struct<uint16_t, std::map<std::string, sdbus::Variant>>> ele00_models = {{0x1000, {}}, {0x1001, {}}};
    std::vector<sdbus::Struct<uint16_t, uint16_t, std::map<std::string, sdbus::Variant>>> ele00_vendormodels = {};
    auto ele00 = std::make_unique<ElementAdaptor>(*bus, obj_ele00, 0, ele00_models, ele00_vendormodels);

    const char* destinationName = "org.bluez.mesh";
    const char* objectPath = "/org/bluez/mesh";
    NetworkProxy networkProxy(*bus, destinationName, objectPath);

    try {
      if(vm.count("join")) {
        uuids::random_generator gen;
        uuids::uuid id = gen();
        std::vector<uint8_t> idvect(id.size());
        std::copy(id.begin(), id.end(), idvect.begin());
        std::cerr << "Joining network, use the provisioning tool of your BLE mesh to provision node " << id << std::endl;
        networkProxy.Join(obj_manager, idvect);
        std::cerr << "Joined the network" << std::endl;
      } else if(vm.count("leave")) {
        std::cerr << "Leaving network" << std::endl;
        networkProxy.Leave(vm["leave"].as<uint64_t>());
        std::cerr << "Left the network" << std::endl;
      } else if(vm.count("attach")) {
        std::cerr << "Attaching node" << std::endl;
        auto reply = networkProxy.Attach(obj_manager, vm["attach"].as<uint64_t>());
        std::string nodePath = std::get<0>(reply);
        NodeProxy node(*bus, destinationName, nodePath);
        std::cerr << "Attached node, bluez DBUS object is " << nodePath << std::endl;

        std::string socket_name = vm["socket"].as<std::string>();
        std::cerr << "Listen on datagram socket " << socket_name << std::endl;
        ::unlink(socket_name.c_str());
        asio::io_service io_service;
        asio::local::datagram_protocol::endpoint dgram_ep(socket_name);
        asio::local::datagram_protocol::endpoint dgram_sender;
        asio::local::datagram_protocol::socket dgram_socket(io_service, dgram_ep);
        char recv_buf[1024];

        std::map<std::string, sdbus::Variant> send_options = {{"ForceSegmented", true}};
        while(true) {
          size_t len = dgram_socket.receive_from(asio::buffer(recv_buf, 1024), dgram_sender);
          std::cerr << "received datagram, size=" << len << std::endl;
          if(len > 0) {
            if(recv_buf[0] == '>') {
              if(len >= (1+2+2)) {
                size_t payload_len = len - 5;
                std::vector<uint8_t> payload(&recv_buf[5], &recv_buf[5] + payload_len);
                uint16_t destination = *(uint16_t*)&recv_buf[1];
                uint16_t key_index = *(uint16_t*)&recv_buf[3];
                std::cerr << "sending message of " << payload_len << "bytes length to destination " << destination << " using appkey " << key_index << std::endl;
                try {
                  node.Send(obj_ele00, destination, key_index, send_options, payload);
                } catch(const sdbus::Error& e) {
                  std::cerr << "error sending message: " << e.getMessage() << " (" << e.getName() << ")" << std::endl;
                }
              } else {
                std::cerr << "sending message: bad format" << std::endl;
              }
            } else {
              std::cerr << "invalid command" << std::endl;
            }
          }
        }
      }
    } catch(const sdbus::Error& e) {
      std::cerr << "Error: " << e.getName() << ", message: " << e.getMessage() << std::endl;
    } catch(const std::exception& e) {
      std::cerr << "Error: " << e.what() << std::endl;
    }
}

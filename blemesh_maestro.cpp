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

#include <boost/algorithm/hex.hpp>

#include <fmt/core.h>
#include <scn/scn.h>

const std::string obj_manager { "/de/hilses/blemesh_maestro" };
const std::string obj_app { "/de/hilses/blemesh_maestro/application" };
const std::string obj_agent { "/de/hilses/blemesh_maestro/agent" };
const std::string obj_ele00 { "/de/hilses/blemesh_maestro/application/ele00" };
const std::string bluez_destination { "org.bluez.mesh" };
const std::string bluez_object { "/org/bluez/mesh" };

std::atomic<bool> running = true;

// adaptor classes for DBUS

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
      running = false;
    }

    void JoinFailed(const std::string& reason) override {
      std::cerr << "Join failed, reason: " << reason << std::endl;
      running = false;
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
      std::cout << fmt::format("< {:04x} {:04x} ", source, key_index);
      // TODO: how to output something sensible for destination argument? (and is it even useful?)
      for(const auto& b: data) std::cout << fmt::format("{:02x}", b);
      std::cout << std::endl;
    }
    void DevKeyMessageReceived(const uint16_t& source, const bool& remote, const uint16_t& net_index, const std::vector<uint8_t>& data) override {
      std::cerr << "Element: DevKeyMessageReceived" << std::endl;
      std::cout << fmt::format("D< {:04x} {} {:04x} ", source, remote, net_index);
      for(const auto& b: data) std::cout << fmt::format("{:02x}", b);
      std::cout << std::endl;
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

// object proxy classes for DBUS

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

// our own stuff

const std::map<std::string, sdbus::Variant> send_options = {{"ForceSegmented", true}};

void node_send(NodeProxy& node, uint16_t destination, uint16_t key_index, std::vector<uint8_t>& msg_bytes) {
  std::cerr << fmt::format("Sending message to {:04x} using key {}: ", destination, key_index);
  for(const auto& b: msg_bytes) std::cerr << fmt::format("{:02x}", b);
  std::cerr << std::endl;
  try {
    node.Send(obj_ele00, destination, key_index, send_options, msg_bytes);
  } catch(const sdbus::Error& e) {
    std::cerr << "Error sending message: " << e.getMessage() << " (" << e.getName() << ")" << std::endl;
  }
}

// for the following client models see the BLE Mesh Model Specification (v. 1.0) to make sense of the message structure

// generic onoff model
void generic_onoff(const std::string& cmdline, NodeProxy& node) {
  static uint8_t next_tid = 0;
  uint16_t destination;
  uint16_t key_index;
  auto result = scn::scan(cmdline, "{:x}", destination);
  if(result) result = scn::scan(result.range(), "{}", key_index);
  std::string command;
  if(result) result = scn::scan(result.range(), "{}", command);
  if(result) {
    if(command == "get") {
      std::vector<uint8_t> msg = {0x82, 0x01};
      node_send(node, destination, key_index, msg);
    } else if(command == "set" || command == "set_unack") {
      uint8_t value;
      result = scn::scan(result.range(), "{}", value);
      if(!result) {
        // TODO: error msg
        return;
      }
      int new_tid = -1;
      result = scn::scan(result.range(), "{}", new_tid);
      if(result) {
        if(new_tid >= 0) {
          next_tid = new_tid;
        } else if(new_tid == -1) {
          next_tid++;
        }
      } else {
        next_tid++;
      }
      std::vector<uint8_t> msg = {0x82, uint8_t(command == "set" ? 0x02 : 0x03), value, next_tid};
      if(result) {
        uint8_t steps;
        uint8_t res = 0;
        uint8_t delay = 0;
        result = scn::scan(result.range(), "{}", steps);
        if(result) {
          result = scn::scan(result.range(), "{}", res);
          if(result) result = scn::scan(result.range(), "{}", delay);
          msg.push_back((res << 6) | steps);
          msg.push_back(delay);
        }
      }
      node_send(node, destination, key_index, msg);
    } else {
      std::cerr << "available subcommands: get, set, set_unack" << std::endl;
    }
  } else {
    std::cerr << "usage: onoff <destination> <key_index> <subcommand> <subcommand arguments>" << std::endl;
  }
}

// generic level model
void generic_level(const std::string& cmdline, NodeProxy& node) {
  static uint8_t next_tid = 0;
  uint16_t destination;
  uint16_t key_index;
  auto result = scn::scan(cmdline, "{:x}", destination);
  if(result) result = scn::scan(result.range(), "{}", key_index);
  std::string command;
  if(result) result = scn::scan(result.range(), "{}", command);
  if(result) {
    if(command == "get") {
      std::vector<uint8_t> msg = {0x82, 0x05};
      node_send(node, destination, key_index, msg);
    } else if(command == "set" || command == "set_unack" || command == "delta" || command == "delta_unack" || command == "move" || command == "move_unack") {
      std::vector<uint8_t> msg = {0x82};
      if(command == "set") { msg.push_back(uint8_t(0x06)); }
      else if(command == "set_unack") { msg.push_back(uint8_t(0x07)); }
      else if(command == "delta") { msg.push_back(uint8_t(0x09)); }
      else if(command == "delta_unack") { msg.push_back(uint8_t(0x0A)); }
      else if(command == "move") { msg.push_back(uint8_t(0x0B)); }
      else if(command == "move_unack") { msg.push_back(uint8_t(0x0C)); }

      if(command == "set" || command == "set_unack" || command == "move" || command == "move_unack") {
        int16_t value;
        result = scn::scan(result.range(), "{}", value);
        if(!result) {
          // TODO: error msg
          return;
        }
        msg.push_back(uint8_t(value & 0xFF));
        msg.push_back(uint8_t(value >> 8));
      } else if(command == "delta" || command == "delta_unack") {
        int32_t value;
        result = scn::scan(result.range(), "{}", value);
        if(!result) {
          // TODO: error msg
          return;
        }
        msg.push_back(uint8_t(value & 0xFF));
        msg.push_back(uint8_t((value >> 8) & 0xFF));
        msg.push_back(uint8_t((value >> 16) & 0xFF));
        msg.push_back(uint8_t(value >> 24));
      }

      int new_tid = -1;
      result = scn::scan(result.range(), "{}", new_tid);
      if(result) {
        if(new_tid >= 0) {
          next_tid = new_tid;
        } else if(new_tid == -1) {
          next_tid++;
        }
      } else {
        next_tid++;
      }
      msg.push_back(uint8_t(next_tid));

      if(result) {
        uint8_t steps;
        uint8_t res = 0;
        uint8_t delay = 0;
        result = scn::scan(result.range(), "{}", steps);
        if(result) {
          result = scn::scan(result.range(), "{}", res);
          if(result) result = scn::scan(result.range(), "{}", delay);
          msg.push_back((res << 6) | steps);
          msg.push_back(delay);
        }
      }
      node_send(node, destination, key_index, msg);
    } else {
      std::cerr << "available subcommands: get, set, set_unack, delta, delta_unack, move, move_unack" << std::endl;
    }
  } else {
    std::cerr << "usage: level <destination> <key_index> <subcommand> <subcommand arguments>" << std::endl;
  }
}

void handle_input(const std::string& cmdline, NodeProxy& node) {
  std::string command;
  auto result = scn::scan(cmdline, "{}", command);
  if(result) {
    if(command == "send") {
      uint16_t destination;
      uint16_t key_index;
      result = scn::scan(result.range(), "{:x}", destination);
      if(result) result = scn::scan(result.range(), "{}", key_index);
      scn::string_view msg;
      if(result) {
        std::vector<uint8_t> msg_bytes;
        while(result) {
          uint8_t b;
          result = scn::scan(result.range(), " {:2x}", b);
          if(result) msg_bytes.push_back(b);
        }
        node_send(node, destination, key_index, msg_bytes);
      } else {
        std::cerr << "Bad syntax, use 'send <dest> <key_index> <data>'" << std::endl;
      }
    } else if(command == "onoff") {
      generic_onoff(result.range_as_string(), node);
    } else if(command == "level") {
      generic_level(result.range_as_string(), node);
    } else if(command == "quit") {
      running = false;
    } else {
      std::cerr << "available commands: send, onoff, level, quit" << std::endl;
    }
  }
}

int main(int argc, char *argv[])
{
    po::options_description desc{"Options"};
    desc.add_options()
      ("join,j", "join a new node")
      ("leave,l", po::value<uint64_t>(), "node identified by given token leaves the network")
      ("attach,a", po::value<uint64_t>(), "node identified by given token attaches to the network, listen on socket for commands")
      ("socket,s", po::value<std::string>(), "name of datagram UNIX socket that will listen for commands");

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

    NetworkProxy networkProxy(*bus, bluez_destination, bluez_object);

    try {
      if(vm.count("join")) {
        uuids::random_generator gen;
        uuids::uuid id = gen();
        std::vector<uint8_t> idvect(id.size());
        std::copy(id.begin(), id.end(), idvect.begin());
        std::cerr << "Joining network, use the provisioning tool of your BLE mesh to provision node " << id << " (";
        for(const auto& b: idvect) std::cerr << fmt::format("{:02x}", b);
        std::cerr << ")" << std::endl;
        networkProxy.Join(obj_manager, idvect);
        while(running) { sleep(1); }
      } else if(vm.count("leave")) {
        std::cerr << "Leaving network" << std::endl;
        networkProxy.Leave(vm["leave"].as<uint64_t>());
        std::cerr << "Left the network" << std::endl;
      } else if(vm.count("attach")) {
        std::cerr << "Attaching node" << std::endl;
        auto reply = networkProxy.Attach(obj_manager, vm["attach"].as<uint64_t>());
        std::string nodePath = std::get<0>(reply);
        NodeProxy node(*bus, bluez_destination, nodePath);
        std::cerr << "Attached node, bluez DBUS object is " << nodePath << std::endl;

        std::string cmdline;
        if(vm.count("socket")) {
          std::string socket_name = vm["socket"].as<std::string>();
          std::cerr << "Listen for commands on datagram socket " << socket_name << std::endl;
          ::unlink(socket_name.c_str());
          asio::io_service io_service;
          asio::local::datagram_protocol::endpoint dgram_ep(socket_name);
          asio::local::datagram_protocol::endpoint dgram_sender;
          asio::local::datagram_protocol::socket dgram_socket(io_service, dgram_ep);
          char recv_buf[1024];

          while(running) {
            size_t len = dgram_socket.receive_from(asio::buffer(recv_buf, 1024), dgram_sender);
            std::cerr << "Received datagram, size=" << len << std::endl;
            cmdline.assign(recv_buf, len);
            handle_input(cmdline, node);
          }
        } else {
          std::cerr << "Listen for commands" << std::endl;
          while(running && std::getline(std::cin, cmdline)) handle_input(cmdline, node);
        }
      }
    } catch(const sdbus::Error& e) {
      std::cerr << "Error: " << e.getName() << ", message: " << e.getMessage() << std::endl;
    } catch(const std::exception& e) {
      std::cerr << "Error: " << e.what() << std::endl;
    }

    bus->leaveEventLoop();
}

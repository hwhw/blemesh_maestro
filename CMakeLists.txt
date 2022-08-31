cmake_minimum_required(VERSION 3.10)
project(BLEMeshMaestro VERSION 0.0.1)

add_executable(blemesh_maestro
  blemesh_maestro.cpp
  org-bluez-mesh-network1-client-glue.h
  org-bluez-mesh-node1-client-glue.h
  org-bluez-mesh-application1-server-glue.h
  org-bluez-mesh-element1-server-glue.h
  org-bluez-mesh-provisionagent1-server-glue.h
)

find_package(Boost REQUIRED COMPONENTS
  program_options)

find_package(sdbus-c++-tools REQUIRED)

find_package(scn CONFIG REQUIRED)

find_package(fmt REQUIRED)

target_link_libraries(blemesh_maestro sdbus-c++ Boost::program_options scn::scn fmt::fmt)

# Use the sdbus-c++-xml2cpp in SDBusCpp namespace to generate the headers
add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/org-bluez-mesh-network1-client-glue.h
    COMMAND SDBusCpp::sdbus-c++-xml2cpp ${PROJECT_SOURCE_DIR}/xml/Network1.xml --proxy=${PROJECT_SOURCE_DIR}/org-bluez-mesh-network1-client-glue.h
    DEPENDS xml/Network1.xml
    COMMENT "Generating D-Bus interfaces for ${PROJECT_NAME}."
)
add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/org-bluez-mesh-node1-client-glue.h
    COMMAND SDBusCpp::sdbus-c++-xml2cpp ${PROJECT_SOURCE_DIR}/xml/Node1.xml --proxy=${PROJECT_SOURCE_DIR}/org-bluez-mesh-node1-client-glue.h
    DEPENDS xml/Node1.xml
    COMMENT "Generating D-Bus interfaces for ${PROJECT_NAME}."
)
add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/org-bluez-mesh-application1-server-glue.h
    COMMAND SDBusCpp::sdbus-c++-xml2cpp ${PROJECT_SOURCE_DIR}/xml/Application1.xml --adapter=${PROJECT_SOURCE_DIR}/org-bluez-mesh-application1-server-glue.h
    DEPENDS xml/Application1.xml
    COMMENT "Generating D-Bus interfaces for ${PROJECT_NAME}."
)
add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/org-bluez-mesh-element1-server-glue.h
    COMMAND SDBusCpp::sdbus-c++-xml2cpp ${PROJECT_SOURCE_DIR}/xml/Element1.xml --adapter=${PROJECT_SOURCE_DIR}/org-bluez-mesh-element1-server-glue.h
    DEPENDS xml/Element1.xml
    COMMENT "Generating D-Bus interfaces for ${PROJECT_NAME}."
)
add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/org-bluez-mesh-provisionagent1-server-glue.h
    COMMAND SDBusCpp::sdbus-c++-xml2cpp ${PROJECT_SOURCE_DIR}/xml/ProvisionAgent1.xml --adapter=${PROJECT_SOURCE_DIR}/org-bluez-mesh-provisionagent1-server-glue.h
    DEPENDS xml/ProvisionAgent1.xml
    COMMENT "Generating D-Bus interfaces for ${PROJECT_NAME}."
)

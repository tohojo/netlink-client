cmake_minimum_required(VERSION 2.8)
project(netlink-client)
find_package(PkgConfig)
PKG_CHECK_MODULES(LIBNL3 REQUIRED libnl-3.0>=3.1)
PKG_CHECK_MODULES(LIBNL3-ROUTE REQUIRED libnl-route-3.0>=3.1)


set(netlink-client_SOURCES
  src/main.c
  src/netlink_comm.c
  src/options.c)
set(netlink-client_HEADERS
  include/linux/rtnetlink.h
  include/netlink_comm.h
  include/options.h)

include_directories(include/ ${LIBNL3_INCLUDE_DIRS})

add_executable(netlink-client
  ${netlink-client_HEADERS}
  ${netlink-client_SOURCES})
target_link_libraries(netlink-client ${LIBNL3_LIBRARIES} ${LIBNL3-ROUTE_LIBRARIES})

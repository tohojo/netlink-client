cmake_minimum_required(VERSION 2.8)
project(qstatsc)
find_package(PkgConfig)
PKG_CHECK_MODULES(LIBNL3 REQUIRED libnl-3.0>=3.1)
PKG_CHECK_MODULES(LIBNL3-ROUTE REQUIRED libnl-route-3.0>=3.1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-long-long")


set(qstatsc_SOURCES
  src/main.c
  src/netlink_comm.c
  src/options.c
  src/qdisc_fq_codel.c
  src/formatter.c
  src/csv_formatter.c)
set(qstatsc_HEADERS
  include/linux/rtnetlink.h
  include/linux/pkt_sched.h
  include/netlink_comm.h
  include/options.h
  include/formatter.h)

include_directories(include/ ${LIBNL3_INCLUDE_DIRS})

add_executable(qstatsc
  ${qstatsc_HEADERS}
  ${qstatsc_SOURCES})
target_link_libraries(qstatsc ${LIBNL3_LIBRARIES} ${LIBNL3-ROUTE_LIBRARIES} dl)

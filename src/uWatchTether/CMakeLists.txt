#--------------------------------------------------------
# The CMakeLists.txt for:                       uWatchTether
# Author(s):                              Mathew Schwartzman
#--------------------------------------------------------

SET(SRC
  WatchTether.cpp
  WatchTether_Info.cpp
  main.cpp
)

ADD_EXECUTABLE(uWatchTether ${SRC})

TARGET_LINK_LIBRARIES(uWatchTether
   ${MOOS_LIBRARIES}
   apputil
   mbutil
   m
   pthread)


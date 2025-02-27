#--------------------------------------------------------
# The CMakeLists.txt for:                       pOdometry
# Author(s):                              Mathew Schwartzman
#--------------------------------------------------------

SET(SRC
  Odometry.cpp
  Odometry_Info.cpp
  NavRecord.cpp
  main.cpp
)

ADD_EXECUTABLE(pOdometry ${SRC})

TARGET_LINK_LIBRARIES(pOdometry
   ${MOOS_LIBRARIES}
   apputil
   mbutil
   m
   pthread)


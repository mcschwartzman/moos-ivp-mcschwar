#--------------------------------------------------------
# The CMakeLists.txt for:                   lib_bhv_tether
# Author(s):                                   M.Schwartzman
#--------------------------------------------------------
#                                               BHV_Tether
#--------------------------------------------------------
ADD_LIBRARY(BHV_Tether SHARED 
   BHV_Tether.cpp)
TARGET_LINK_LIBRARIES(BHV_Tether
   -Wl,--whole-archive dep_behaviors -Wl,--no-whole-archive
   -Wl,--whole-archive contacts -Wl,--no-whole-archive
   -Wl,--whole-archive mbutil -Wl,--no-whole-archive
   helmivp
   behaviors 
   ivpbuild 
   logic 
   ivpcore 
   bhvutil 
   geometry 
)



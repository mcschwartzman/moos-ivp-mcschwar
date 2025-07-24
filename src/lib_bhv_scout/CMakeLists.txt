#--------------------------------------------------------
# The CMakeLists.txt for:                   lib_bhv_scout
# Author(s):                                   M.Benjamin
#--------------------------------------------------------
#                                               BHV_Scout
#--------------------------------------------------------
ADD_LIBRARY(BHV_Scout SHARED 
   BHV_Scout.cpp)
TARGET_LINK_LIBRARIES(BHV_Scout
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



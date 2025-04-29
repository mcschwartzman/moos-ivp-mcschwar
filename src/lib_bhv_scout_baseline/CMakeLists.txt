#--------------------------------------------------------
# The CMakeLists.txt for:                   lib_bhv_scout
# Author(s):                                   M.Benjamin
#--------------------------------------------------------
#                                               BHV_Scout
#--------------------------------------------------------
ADD_LIBRARY(BHV_Scout_baseline SHARED 
   BHV_Scout_baseline.cpp)
TARGET_LINK_LIBRARIES(BHV_Scout_baseline
   helmivp
   behaviors 
   ivpbuild 
   logic 
   ivpcore 
   bhvutil 
   mbutil 
   geometry 
)



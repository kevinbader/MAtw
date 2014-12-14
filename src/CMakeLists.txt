## 
## Copyright 2014 Kevin Bader.
## Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
## 
file(GLOB LIBD2_CPP "*.cpp")
add_library(libd2 ${LIBD2_CPP})
target_link_libraries(libd2 sqlite ${Boost_LIBRARIES})

remove_definitions(
    -std=c++11
    -Wall
    -Wextra
    -Woverloaded-virtual
)
add_library(sqlite sqlite3.c)
target_link_libraries(sqlite pthread dl)

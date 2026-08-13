FILE(GLOB_RECURSE HEADERS "*.h")
add_library(NlohmannJson INTERFACE ${HEADER})
target_include_directories(NlohmannJson INTERFACE include)

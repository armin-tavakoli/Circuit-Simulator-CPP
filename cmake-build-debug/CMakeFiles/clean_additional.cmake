# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\circuit_simulator_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\circuit_simulator_autogen.dir\\ParseCache.txt"
  "circuit_simulator_autogen"
  )
endif()

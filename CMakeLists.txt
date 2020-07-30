
cmake_minimum_required(VERSION 2.6)
project (tsa5013)

#Can manually add the sources using the set command as follows:
set(SOURCES TCA5013_test.c TCA5013.c cubic_I2C.c)

#add include_directories
include_directories(/usr/local/include/periphery)

#add lib_directories
link_directories("/usr/local/lib64/")

#add lib
link_libraries(libperiphery.so)


add_executable(tsa5013_test ${SOURCES})

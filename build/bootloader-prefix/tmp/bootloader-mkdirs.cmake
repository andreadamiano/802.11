# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/andrea/.espressif/v6.0/esp-idf/components/bootloader/subproject"
  "/home/andrea/projects/Software/Wifi/build/bootloader"
  "/home/andrea/projects/Software/Wifi/build/bootloader-prefix"
  "/home/andrea/projects/Software/Wifi/build/bootloader-prefix/tmp"
  "/home/andrea/projects/Software/Wifi/build/bootloader-prefix/src/bootloader-stamp"
  "/home/andrea/projects/Software/Wifi/build/bootloader-prefix/src"
  "/home/andrea/projects/Software/Wifi/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/andrea/projects/Software/Wifi/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/andrea/projects/Software/Wifi/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()

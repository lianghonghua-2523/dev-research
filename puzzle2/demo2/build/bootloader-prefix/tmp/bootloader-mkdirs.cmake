# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/Espressif/esp-idf/components/bootloader/subproject"
  "D:/Users/lianghonghua/Github_Sourse/interv/dev-research/puzzle2/demo2/build/bootloader"
  "D:/Users/lianghonghua/Github_Sourse/interv/dev-research/puzzle2/demo2/build/bootloader-prefix"
  "D:/Users/lianghonghua/Github_Sourse/interv/dev-research/puzzle2/demo2/build/bootloader-prefix/tmp"
  "D:/Users/lianghonghua/Github_Sourse/interv/dev-research/puzzle2/demo2/build/bootloader-prefix/src/bootloader-stamp"
  "D:/Users/lianghonghua/Github_Sourse/interv/dev-research/puzzle2/demo2/build/bootloader-prefix/src"
  "D:/Users/lianghonghua/Github_Sourse/interv/dev-research/puzzle2/demo2/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Users/lianghonghua/Github_Sourse/interv/dev-research/puzzle2/demo2/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
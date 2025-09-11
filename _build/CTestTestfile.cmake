# CMake generated Testfile for 
# Source directory: /home/runner/work/sedflux/sedflux
# Build directory: /home/runner/work/sedflux/sedflux/_build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(Help "/home/runner/work/sedflux/sedflux/_build/ew/sedflux/run_sedflux" "--help")
set_tests_properties(Help PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/sedflux/sedflux/CMakeLists.txt;11;add_test;/home/runner/work/sedflux/sedflux/CMakeLists.txt;0;")
add_test(SedCell "gtester" "/home/runner/work/sedflux/sedflux/_build/ew/sed/sed-test-cell")
set_tests_properties(SedCell PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/sedflux/sedflux/CMakeLists.txt;12;add_test;/home/runner/work/sedflux/sedflux/CMakeLists.txt;0;")
add_test(SedColumn "gtester" "/home/runner/work/sedflux/sedflux/_build/ew/sed/sed-test-column")
set_tests_properties(SedColumn PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/sedflux/sedflux/CMakeLists.txt;13;add_test;/home/runner/work/sedflux/sedflux/CMakeLists.txt;0;")
add_test(SedCube "gtester" "/home/runner/work/sedflux/sedflux/_build/ew/sed/sed-test-cube")
set_tests_properties(SedCube PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/sedflux/sedflux/CMakeLists.txt;14;add_test;/home/runner/work/sedflux/sedflux/CMakeLists.txt;0;")
add_test(SedHydro "gtester" "/home/runner/work/sedflux/sedflux/_build/ew/sed/sed-test-hydro")
set_tests_properties(SedHydro PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/sedflux/sedflux/CMakeLists.txt;15;add_test;/home/runner/work/sedflux/sedflux/CMakeLists.txt;0;")
add_test(SedRiver "gtester" "/home/runner/work/sedflux/sedflux/_build/ew/sed/sed-test-river")
set_tests_properties(SedRiver PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/sedflux/sedflux/CMakeLists.txt;16;add_test;/home/runner/work/sedflux/sedflux/CMakeLists.txt;0;")
add_test(SedWave "gtester" "/home/runner/work/sedflux/sedflux/_build/ew/sed/sed-test-wave")
set_tests_properties(SedWave PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/sedflux/sedflux/CMakeLists.txt;17;add_test;/home/runner/work/sedflux/sedflux/CMakeLists.txt;0;")
add_test(UtilsGrid "gtester" "/home/runner/work/sedflux/sedflux/_build/ew/utils/utils-test-grid")
set_tests_properties(UtilsGrid PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/sedflux/sedflux/CMakeLists.txt;18;add_test;/home/runner/work/sedflux/sedflux/CMakeLists.txt;0;")
add_test(UtilsIO "gtester" "/home/runner/work/sedflux/sedflux/_build/ew/utils/utils-test-io")
set_tests_properties(UtilsIO PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/sedflux/sedflux/CMakeLists.txt;19;add_test;/home/runner/work/sedflux/sedflux/CMakeLists.txt;0;")
add_test(UtilsKeyFile "gtester" "/home/runner/work/sedflux/sedflux/_build/ew/utils/utils-test-key-file")
set_tests_properties(UtilsKeyFile PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/sedflux/sedflux/CMakeLists.txt;20;add_test;/home/runner/work/sedflux/sedflux/CMakeLists.txt;0;")
add_test(UtilsNum "gtester" "/home/runner/work/sedflux/sedflux/_build/ew/utils/utils-test-num")
set_tests_properties(UtilsNum PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/sedflux/sedflux/CMakeLists.txt;21;add_test;/home/runner/work/sedflux/sedflux/CMakeLists.txt;0;")
add_test(UtilsSymbolTable "gtester" "/home/runner/work/sedflux/sedflux/_build/ew/utils/utils-test-symbol-table")
set_tests_properties(UtilsSymbolTable PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/sedflux/sedflux/CMakeLists.txt;22;add_test;/home/runner/work/sedflux/sedflux/CMakeLists.txt;0;")
subdirs("ew")

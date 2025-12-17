add_executable(EngineTests MatchingEngineTest.cpp ObjectPoolTest.cpp)

target_link_libraries(EngineTests PRIVATE 
    MatchingCore 
    GTest::gtest 
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(EngineTests)
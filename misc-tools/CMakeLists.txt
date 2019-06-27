add_executable(flexEstimatorTester model-base-flex-estimator-file-tester.cpp)
target_link_libraries(flexEstimatorTester PRIVATE ${PROJECT_NAME} Boost::filesystem)

add_executable(fileSplitter file-splitter.cpp)
target_link_libraries(fileSplitter PRIVATE ${PROJECT_NAME} Boost::filesystem)

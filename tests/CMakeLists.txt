add_executable(kiofilecopyjobtest
    kiofilecopyjobtest.cpp)
target_link_libraries(kiofilecopyjobtest
    Qt5::Test
    KF5::KIOCore)
add_test(kiofilecopyjobtest kiofilecopyjobtest)

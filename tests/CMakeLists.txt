add_executable(test_tagengine
    test_tagengine.cpp
    ../src/tagengine.cpp
)

target_include_directories(test_tagengine PRIVATE ../src)
target_link_libraries(test_tagengine PRIVATE
    Qt6::Core
    Qt6::Xml
    Qt6::Test
)

add_test(NAME test_tagengine COMMAND test_tagengine)

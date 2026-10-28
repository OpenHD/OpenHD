find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# A library inside the existing core executable, with no separate application.
add_library(openhd_gui STATIC openhd_gui.cpp wifi_test.qrc)
set_target_properties(openhd_gui PROPERTIES AUTORCC ON)
if(CMAKE_CROSSCOMPILING)
    find_program(OPENHD_HOST_RCC NAMES rcc rcc-qt5
        PATHS /usr/lib/qt${QT_VERSION_MAJOR}/bin /usr/lib/qt${QT_VERSION_MAJOR}/libexec
        NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH REQUIRED)
    set_target_properties(openhd_gui PROPERTIES AUTORCC_EXECUTABLE "${OPENHD_HOST_RCC}")
endif()
target_include_directories(openhd_gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(openhd_gui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets openhd_wifi_test)
target_compile_definitions(openhd_gui PUBLIC OPENHD_ENABLE_GUI PRIVATE QT_NO_KEYWORDS)

add_executable(test_wifi_test_gui EXCLUDE_FROM_ALL test_wifi_test_gui.cpp)
if(NOT OPENHD_PORTABLE_GROUND)
    target_sources(test_wifi_test_gui PRIVATE ../ohd_common/src/config_paths.cpp)
endif()
target_link_libraries(test_wifi_test_gui PRIVATE openhd_gui openhd_wifi_test Qt${QT_VERSION_MAJOR}::Widgets)
add_test(NAME wifi_test_gui COMMAND test_wifi_test_gui)
set_tests_properties(wifi_test_gui PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 20)

if(WIN32)
    target_sources(openhd_gui PRIVATE windows_usb_driver_gui.cpp)
    target_link_libraries(openhd_gui PRIVATE shell32)
endif()

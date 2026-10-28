find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# A library inside the existing core executable, with no separate application.
add_library(openhd_gui STATIC openhd_gui.cpp)
target_include_directories(openhd_gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(openhd_gui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_compile_definitions(openhd_gui PUBLIC OPENHD_ENABLE_GUI PRIVATE QT_NO_KEYWORDS)

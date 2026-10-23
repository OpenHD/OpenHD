cmake_minimum_required(VERSION 3.16)
project(pdcursesmod C)

# Wide-character Win32 console backend, matching MSYS2's pdcurses_wincon.
# common/*.c are #included by the wincon sources, not compiled on their own.
set(src ${CMAKE_CURRENT_SOURCE_DIR}/..)
file(GLOB pdcurses_core ${src}/pdcurses/*.c)
file(GLOB pdcurses_wincon ${src}/wincon/*.c)
add_library(pdcurses_wincon ${pdcurses_core} ${pdcurses_wincon})
target_include_directories(pdcurses_wincon PUBLIC
    $<BUILD_INTERFACE:${src}>
    $<INSTALL_INTERFACE:include/pdcurses>)
target_compile_definitions(pdcurses_wincon PUBLIC PDC_WIDE PDC_FORCE_UTF8)
if(BUILD_SHARED_LIBS)
    target_compile_definitions(pdcurses_wincon PUBLIC PDC_DLL_BUILD)
endif()
target_link_libraries(pdcurses_wincon PUBLIC user32 winmm)

install(TARGETS pdcurses_wincon EXPORT unofficial-pdcurses-targets)
install(FILES ${src}/curses.h ${src}/panel.h ${src}/term.h DESTINATION include/pdcurses)
install(EXPORT unofficial-pdcurses-targets
    NAMESPACE unofficial::pdcurses::
    FILE unofficial-pdcurses-config.cmake
    DESTINATION share/unofficial-pdcurses)

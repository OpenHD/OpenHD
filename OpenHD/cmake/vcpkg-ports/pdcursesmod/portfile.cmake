vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Bill-Gray/PDCursesMod
    REF v${VERSION}
    SHA512 37c05a24c041c7ce1f159f3dd9352a4317f75f32ee32f66d35a102d73cb95eb81c7381c880b43c48e414544360efe946204712238a7e0c81cbfa9fa85add97d6
    HEAD_REF master
)
# Upstream's CMakeLists builds every port and demo; ours builds only wincon.
file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}/vcpkg")

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}/vcpkg")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME unofficial-pdcurses CONFIG_PATH share/unofficial-pdcurses)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/README.md")

# OpenHD Buildroot Package

This package installs OpenHD on Buildroot. 

## Requirements

One key requirement is the use of **C++17**, ensuring compatibility with modern C++ standards. As a result, older libraries like `libpoco` must be updated from the `UpdatedDependencies` repository.

Please follow the instructions below to ensure all required dependencies are installed and properly configured. *(Note: This list may change over time.)*

## Dependencies

To build and use this package, ensure you have the following dependencies enabled in your Buildroot configuration:

- `BR2_PACKAGE_FFMPEG_ARCH_SUPPORTS=y` *(Optional)*
- `BR2_PACKAGE_GSTREAMER1=y`
- `BR2_PACKAGE_GSTREAMER1_PARSE=y`
- `BR2_PACKAGE_GSTREAMER1_TRACE=y`
- `BR2_PACKAGE_GSTREAMER1_PLUGIN_REGISTRY=y`
- `BR2_PACKAGE_GST1_PLUGINS_BASE=y`
- `BR2_PACKAGE_GST1_PLUGINS_BASE_PLUGIN_TYPEFIND=y`
- `BR2_PACKAGE_GST1_PLUGINS_BASE_PLUGIN_VIDEOSCALE=y`
- `BR2_PACKAGE_EXPAT=y`
- `BR2_PACKAGE_POCO=y`
- `BR2_PACKAGE_POCO_NET=y`
- `BR2_PACKAGE_LIBPCAP=y`

These dependencies have been tested, but other configurations may also be possible. Make sure you are using at least these versions to ensure compatibility.

## Development and Future Plans

This package was initially developed for **reCamera-OS**. We plan to extend support in future updates, adding more functionality and improving performance.

## Special Thanks

A huge thank you to **@buldo** for his invaluable help and contributions to the initial package(s).

## Notes

Please note that this README only covers support for the **Buildroot package configuration** itself. Support for hardware setups and configurations that run this package is beyond the scope of this repository.

Feel free to reach out if you have questions regarding the package or installation steps!

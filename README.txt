
This is the CEF project patched with initial Aura window manager and Ozone support.

Building
----------

1. Setup your Chromium/CEF GIT repositories as it is documented on the CEF3
website.

2. Setup an ARM build:

# ./cef_create_projects.sh -I arm_ozone.gypi

Changelog:
----------
- Building for embedded ARM and X86 devices.
- Support for Aura window manager.
- Building CEF without toolkit views.

ToDo:
---------
- Test builds for various platforms supported by CEF.

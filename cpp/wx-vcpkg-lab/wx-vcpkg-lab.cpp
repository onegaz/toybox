#include <iostream>
#include <wx/wx.h>

int main()
{
    wxString wxHelloWorld;
	wxHelloWorld.sprintf(wxT("Hello vcpkg, pid: %lu\n"), wxGetProcessId());
    std::cout << wxHelloWorld;
}

// method 1:
// rm CMakeCache.txt ; VCPKG_TARGET_TRIPLET=x64-osx CMAKE_TOOLCHAIN_FILE=~/oss/vcpkg/scripts/buildsystems/vcpkg.cmake cmake -S .. -B .

// method 2:
// rm CMakeCache.txt ; wxWidgets_INCLUDE_DIRS=~/oss/vcpkg/packages/wxwidgets_x64-osx/include/wx-3.1 wxWidgets_LIBRARIES=~/oss/vcpkg/packages/wxwidgets_x64-osx/lib  cmake -S .. -B .

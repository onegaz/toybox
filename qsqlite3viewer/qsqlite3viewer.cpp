#include <iostream>
#include <fstream>
#include <boost/locale/encoding_utf.hpp>
#include <string>
#include <boost/locale.hpp>
#include <algorithm>
#include <memory>
#include <boost/algorithm/string/predicate.hpp>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>

#if  !defined( __WXGTK__ ) && !defined(_MSC_VER)
#include "mainwnd.h"
#endif
int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    MainWnd w;
    w.show();
    return a.exec();
}

#include <boost/predef.h>
#include <pngwriter.h>
#include <boost/program_options.hpp>
#include <deque>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include "text_to_img.hpp"
#include "pngwriter_text_to_img.hpp"

void pngwriter_text_to_img::convert(
    const std::string& ttf, int width, int font_size, int line_space,
    const std::deque<std::string>& lines, const std::string& outfile)
{
  int height = (font_size + line_space) * lines.size() + line_space;
  int top = height;
  int backgroundcolour = 65535;
  pngwriter one(width, height, backgroundcolour, outfile.c_str());
  for (auto& aline : lines) {
    top -= font_size + line_space;
    if (top < 0) break;

    one.plot_text_utf8(const_cast<char*>(ttf.c_str()), font_size, 10, top, 0,
                       const_cast<char*>(aline.c_str()), 0.0, 0.0, 1.0);
  }
  one.close();
}

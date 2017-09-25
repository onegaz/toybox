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
// dependencies on MacOS
// https://github.com/pngwriter/pngwriter
// brew install freetype
// alternative boost::freetype - extension for boost::gil

void txt_to_lines(const std::string& line, int num_per_line, std::deque<std::string>& lines)
{
    int nstart = 0;
    char buf[1024];
    int pos = 0;
    int num = 0;
    auto append_line = [&]() {
        buf[nstart] = 0;
        lines.emplace_back(buf);
        num = 0;
        nstart = 0;
    };

    while (pos < line.length())
    {
        num++;
        if (line[pos] > 127)
        {
            buf[nstart++] = line[pos++];
            if (pos < line.length())
                buf[nstart++] = line[pos++];
        }
        else
        {
            buf[nstart++] = line[pos++];
        }
        // avoid extreme short line like only Punctuation
        if (nstart >= num_per_line && line.length() - pos > 3)
        {
            append_line();
        }
    }

    if (num > 0)
    {
        append_line();
    }
}

std::deque<std::string> file_to_lines(const std::string& srcfile, int num_per_line)
{
    std::ifstream intext(srcfile.c_str());
    std::string line;
    std::deque<std::string> lines;
    while (std::getline(intext, line))
    {
    	txt_to_lines(line, num_per_line, lines);
    }
    return lines;
}

int main1(int argc, char* argv[])
{
    std::string srcfile;
    std::string outfile = "/tmp/txt2img.png";

    int width = 800;
    int height = 1600;             // it is auto calculated according to text length.
    int backgroundcolour = 65535;  // white
    int font_size = 12;
    int line_space = 4;

#if BOOST_OS_WINDOWS
    const char* font_help = "path of font file";
#elif BOOST_OS_LINUX
    std::string ttf = "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc";
    const char* font_help = "path of font file like /usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc";
#elif BOOST_OS_MACOS
    std::string ttf = "/System/Library/Fonts/STHeiti Light.ttc";
    const char* font_help = "path of font file like /Library/Fonts/Arial.ttf or "
            "/System/Library/Fonts/STHeiti Light.ttc";
#endif

    boost::program_options::variables_map vm;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()("help,h", "produce help message")(
        "srcfile", boost::program_options::value<decltype(srcfile)>(&srcfile),
        "path of input txt file")(
        "outfile", boost::program_options::value<decltype(outfile)>(&outfile),
        "path of output image file")("ttf",
                                     boost::program_options::value<decltype(ttf)>(&ttf),
									 font_help)(
        "font_size", boost::program_options::value<decltype(font_size)>(&font_size),
        "font_size")("line_space",
                     boost::program_options::value<decltype(line_space)>(&line_space),
                     "line_space")(
        "width,w", boost::program_options::value<decltype(width)>(&width), "width")(
        "backgroundcolour,b",
        boost::program_options::value<decltype(backgroundcolour)>(&backgroundcolour),
        "backgroundcolour, white: 65535; black: 0");
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if (vm.count("help") || srcfile.length() < 1)
    {
        std::cout << desc << std::endl;
        return 0;
    }

    int num_per_line = width * 18 / (font_size * 10);
    std::deque<std::string> lines = file_to_lines(srcfile, num_per_line);
    std::unique_ptr<text_to_img> t2i(new pngwriter_text_to_img);
    t2i->convert(ttf, width, font_size, line_space, lines, outfile);

#if BOOST_OS_WINDOWS
    std::cout << "start " << outfile << std::endl;
#elif BOOST_OS_LINUX
    std::cout << "xdg-open " << outfile << std::endl;
#elif BOOST_OS_MACOS
    std::cout << "open " << outfile << std::endl;
#endif
    return 0;
}

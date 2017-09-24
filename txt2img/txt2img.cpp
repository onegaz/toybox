#include <boost/predef.h>
#include <pngwriter.h>
#include <boost/program_options.hpp>
#include <deque>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>

// dependencies on MacOS
// https://github.com/pngwriter/pngwriter
// brew install freetype
// alternative boost::freetype - extension for boost::gil

class text_to_img
{
public:
    virtual void convert(const std::string& ttf, int width, int font_size, int line_space,
                         const std::deque<std::string>& lines,
                         const std::string& outfile) = 0;
    virtual ~text_to_img() = default;
};

class pngwriter_text_to_img : public text_to_img
{
public:
    void convert(const std::string& ttf, int width, int font_size, int line_space,
                 const std::deque<std::string>& lines,
                 const std::string& outfile) override
    {
        int height = (font_size + line_space) * lines.size();
        int top = height;
        int backgroundcolour = 65535;
        pngwriter one(width, height, backgroundcolour, outfile.c_str());
        for (auto& aline : lines)
        {
            top -= font_size + line_space;

            if (top < 0)
                break;

            one.plot_text_utf8(const_cast<char*>(ttf.c_str()), font_size, 10, top, 0,
                               const_cast<char*>(aline.c_str()), 0.0, 0.0, 1.0);
        }
        one.close();
    }
};

int main(int argc, char* argv[])
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

    int top = height;
    std::ifstream intext(srcfile.c_str());
    std::string line;
    int num_per_line = width * 18 / (font_size * 10);
    std::deque<std::string> lines;
    while (std::getline(intext, line))
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
            if (num >= num_per_line && line.length() - pos > 3)
            {
                append_line();
            }
        }

        if (num > 0)
        {
            append_line();
        }
    }
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

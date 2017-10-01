
#include <deque>
#include <memory>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <boost/predef.h>
#include <boost/program_options.hpp>
#include <pngwriter.h>
#include "text_to_img.hpp"
#include "pngwriter_text_to_img.hpp"

void pngwriter_text_to_img::convert(const std::string& ttf, int width, int font_size,
                                    int line_space, const std::deque<std::string>& lines,
                                    const std::string& outfile)
{

    int height = (font_size + line_space) * lines.size() + line_space;
    int top = height;
    pngwriter one(width, height, m_backgroundcolour, outfile.c_str());
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

void pngwriter_text_to_img::txt_to_lines(const std::string& line,
                                         pngwriter* font_width_calc)
{
    int nstart = 0;
    std::array<char, 1024> buf;
    int pos = 0;
    int num = 0;
    auto append_line = [&]() {
        buf[nstart] = 0;
        m_lines.emplace_back(buf.data());
        num = 0;
        nstart = 0;
        buf.fill(0);
    };

    int num_per_line = m_width / m_font_size;

    buf.fill(0);
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

        if (buf[nstart - 1] == '\n')
        {
            append_line();
            continue;
        }

        // avoid extreme short line like only Punctuation
        if (nstart >= num_per_line)
        {
            int cur_width = font_width_calc->get_text_width_utf8(
                const_cast<char*>(m_ttf.c_str()), m_font_size, buf.data());
            if (cur_width + m_font_size * 2 >= m_width)
            {
                append_line();
            }
        }
    }

    if (num > 0)
        append_line();
}

void pngwriter_text_to_img::file_to_lines()
{
    std::ifstream intext(m_txtfile.c_str());
    std::string line;

    pngwriter font_width_calc(m_width, 100, m_backgroundcolour, "tmpfile.png");

    while (std::getline(intext, line))
    {
        txt_to_lines(line, &font_width_calc);
    }
}

pngwriter_text_to_img::pngwriter_text_to_img(const std::string& ttf, int width,
                                             int font_size, int line_space, int bgcolor,
                                             const std::string& txtfile,
                                             const std::string& outfile) :
    m_ttf(ttf),
    m_width(width),
    m_font_size(font_size),
    m_line_space(line_space),
    m_backgroundcolour(bgcolor),
    m_txtfile(txtfile),
    m_outfile(outfile)
{
}

void pngwriter_text_to_img::convert()
{
    file_to_lines();
    convert(m_ttf, m_width, m_font_size, m_line_space, m_lines, m_outfile);
}

void pngwriter_text_to_img::convert(const std::string& line)
{
    std::unique_ptr<pngwriter> pngwriterobj =
        std::make_unique<pngwriter>(m_width, 100, m_backgroundcolour, "tmpfile.png");
    txt_to_lines(line, pngwriterobj.get());
    convert(m_ttf, m_width, m_font_size, m_line_space, m_lines, m_outfile);
}

#ifndef pngwriter_text_to_img_hpp
#define pngwriter_text_to_img_hpp

class pngwriter;

class pngwriter_text_to_img : public text_to_img
{
public:
    pngwriter_text_to_img(const std::string& ttf, int width, int font_size,
                          int line_space, int bgcolor,
						  const std::string& txtfile,
                          const std::string& outfile);

    void convert(const std::string& ttf, int width, int font_size, int line_space,
                 const std::deque<std::string>& lines,
                 const std::string& outfile);


    void file_to_lines();
    void txt_to_lines(const std::string& line, pngwriter* font_width_calc);
    void convert();	// convert txt file
    void convert(const std::string& text); // convert text
    std::string m_ttf;
    int m_width = 960;
    int m_font_size = 12;
    int m_line_space = 4;
    int m_backgroundcolour = 65535;
    std::string m_txtfile;
    std::string m_outfile;
    std::deque<std::string> m_lines;
};

#endif

#ifndef text_to_img_hpp
#define text_to_img_hpp

class text_to_img
{
public:
    virtual void convert(const std::string& ttf, int width, int font_size, int line_space,
                         const std::deque<std::string>& lines,
                         const std::string& outfile) = 0;
    virtual ~text_to_img() = default;
};

std::deque<std::string> file_to_lines(const std::string& srcfile, int num_per_line);
void txt_to_lines(const std::string& src, int num_per_line, std::deque<std::string>& lines);
#endif

#ifndef text_to_img_hpp
#define text_to_img_hpp

class text_to_img
{
public:
    virtual void convert() = 0;                        // convert txt file
    virtual void convert(const std::string& text) = 0; // convert text
    virtual ~text_to_img() = default;
};

std::deque<std::string> file_to_lines(const std::string& srcfile, int num_per_line);
void txt_to_lines(const std::string& src, int num_per_line, std::deque<std::string>& lines);
#endif

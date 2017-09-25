#ifndef pngwriter_text_to_img_hpp
#define pngwriter_text_to_img_hpp

class pngwriter_text_to_img : public text_to_img
{
public:
 void convert(const std::string& ttf, int width, int font_size, int line_space,
              const std::deque<std::string>& lines,
              const std::string& outfile) override;
};

#endif

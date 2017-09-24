all: txt2img


ifndef pngwriter_dir
    pngwriter_dir=/oss/pngwriter
endif

ifndef freetype2_dir
    freetype2_dir=/usr/local/Cellar/freetype/2.8.1
endif

INCLUDES = -I $(pngwriter_dir)/src -I $(freetype2_dir)/include/freetype2

LDFLAGS = -lboost_program_options -L $(pngwriter_dir) -L $(freetype2_dir)/lib -lpngwriter -lfreetype

txt2img: txt2img.cpp
	g++ -o $@ -std=c++11 -pthread $(INCLUDES) $< $(LDFLAGS) -lboost_regex -lboost_exception
	
clean: 
	@if [ -f txt2img ]; then rm txt2img; fi;
TARGET=imgui-boilerplate.a

IMGUI_DIR = imgui

SOURCES = window.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp
SOURCES += $(IMGUI_DIR)/imgui_demo.cpp
SOURCES += $(IMGUI_DIR)/imgui_draw.cpp
SOURCES += $(IMGUI_DIR)/imgui_tables.cpp
SOURCES += $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp

OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
UNAME_S := $(shell uname -s)

CXXFLAGS = -std=c++17 -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS += -Wall -Wformat -Wfatal-errors
CFLAGS   = -std=c99 -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends

##---------------------------------------------------------------------
## OPENGL LOADER / OPENGL ES
##---------------------------------------------------------------------

## Using OpenGL loader: gl3w
SOURCES += imgui/examples/libs/gl3w/GL/gl3w.c
CXXFLAGS += -Iimgui/examples/libs/gl3w -DIMGUI_IMPL_OPENGL_LOADER_GL3W
CFLAGS += -Iimgui/examples/libs/gl3w -DIMGUI_IMPL_OPENGL_LOADER_GL3W

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux) #LINUX
	CXXFLAGS += `sdl2-config --cflags`
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	CXXFLAGS += `sdl2-config --cflags`
	CXXFLAGS += -I/usr/local/include -I/opt/local/include
endif

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: $(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: $(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: imgui/examples/libs/gl3w/GL/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: $(TARGET)

$(TARGET): $(OBJS)
	$(AR) -rcs $(TARGET) $^

clean:
	rm -f $(TARGET) $(OBJS)

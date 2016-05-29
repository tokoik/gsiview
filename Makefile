TARGET	= gsiview
SOURCES	= $(wildcard *.cpp)
HEADERS	= $(wildcard *.h)
OBJECTS	= $(patsubst %.cpp,%.o,$(SOURCES))
LIBOVR	= /usr/local/LibOVR
LIBOCV	= /usr/local
DEBUG	= -g -D_DEBUG
#DEBUG	= -O3
CXXFLAGS	= --std=c++0x -Wall $(DEBUG) -DX11 -I$(LIBOVR)/Include -I$(LIBOCV)/include
LDLIBS	= -lGL -lglfw3 -lXrandr -lXinerama -lXcursor -lXxf86vm -lXi -lX11 -lpthread -lrt -lm -lpng -ludev -L$(LIBOVR)/Lib/Linux/Debug/i386 -lovr -L$(LIBOCV)/lib -lopencv_core -lopencv_highgui -lopencv_imgproc

.PHONY: clean

$(TARGET): $(OBJECTS)
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(TARGET).dep: $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -MM $(SOURCES) > $@

clean:
	-$(RM) $(TARGET) *.o *~ .*~ a.out core

-include $(TARGET).dep

TARGET	= gsiview
SOURCES	= $(wildcard *.cpp)
HEADERS	= $(wildcard *.h)
OBJECTS	= $(patsubst %.cpp,%.o,$(SOURCES))
LIBOCV	= /usr/local
LIBOVR	= /usr/local/LibOVR
DEBUG	= -g -D_DEBUG
#DEBUG	= -O3
CXXFLAGS	= --std=c++0x -Wall $(DEBUG) -Dnullptr=NULL -DX11 -I$(LIBOCV)/include -I$(LIBOVR)/Include
LDLIBS	= -lGL -lglfw3 -lXi -lXrandr -lXxf86vm -lX11 -lrt -lpthread -lpng -ludev -lm -L$(LIBOCV)/lib -lopencv_core -lopencv_highgui -lopencv_imgproc -L$(LIBOVR)/Lib/Linux/Debug/i386 -lovr

.PHONY: clean

$(TARGET): $(OBJECTS)
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(TARGET).dep: $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -MM $(SOURCES) > $@

clean:
	-$(RM) $(TARGET) *.o *~ .*~ a.out core

-include $(TARGET).dep

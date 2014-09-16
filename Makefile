TARGET	= gsiview
SOURCES	= $(wildcard *.cpp)
HEADERS	= $(wildcard *.h)
OBJECTS	= $(patsubst %.cpp,%.o,$(SOURCES))
LIBOCV	= /usr/local
LIBOVR	= /usr/local/LibOVR
DEBUG	= -D_DEBUG
CXXFLAGS	= --std=c++0x -g -Wall -DX11 -I$(LIBOCV)/include -I$(LIBOVR)/Include -Dnullptr=NULL $(DEBUG)
LDLIBS	= -lGL -lglfw3 -lXi -lXrandr -lXxf86vm -lX11 -lrt -lpthread -lpng -ludev -lm -L$(LIBOCV)/lib -lopencv_core -lopencv_highgui -lopencv_imgproc -L$(LIBOVR)/Lib/Linux/Debug/i386 -lovr

.PHONY: clean

$(TARGET): $(OBJECTS)
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(TARGET).dep: $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -MM $(SOURCES) > $@

clean:
	-$(RM) $(TARGET) *.o *~ .*~ a.out core

-include $(TARGET).dep

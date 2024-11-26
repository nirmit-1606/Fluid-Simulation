CXX = g++
CXXFLAGS = -std=c++11 -Wno-deprecated -Xpreprocessor -fopenmp
FRAMEWORKS = -framework OpenGL -framework GLUT
INCLUDES = -Iinclude -I/opt/homebrew/include
LIBS = -L/opt/homebrew/lib -L/opt/homebrew/opt/libomp/lib -lomp libglui.a

fluid: main.cpp
		$(CXX) $(CXXFLAGS) $(FRAMEWORKS) $(INCLUDES) main.cpp -o fluid $(LIBS)

clean:
		rm -f fluid

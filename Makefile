# fluid:		main.cpp
# 		g++ -framework OpenGL -framework GLUT main.cpp -o fluid -I. -Wno-deprecated

fluid2:    main.cpp
		g++ -framework OpenGL -framework GLUT main.cpp -o fluid -I. -Wno-deprecated -Xpreprocessor -fopenmp -I/opt/homebrew/include -L/opt/homebrew/lib -L/opt/homebrew/opt/libomp/lib -lomp

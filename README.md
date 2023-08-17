# CameraCalibration
Chessboard Pattern Detection &amp; Camera Calibration System
- Compile command:
  
	For tasks 1, 2, 3: g++ -std=c++14 task123.cpp $(pkg-config opencv4 --cflags --libs) -o task123

	For tasks 4, 5, 6: g++ -std=c++14 task456.cpp $(pkg-config opencv4 --cflags --libs) -o task456

	For task 7: g++ -std=c++14 task7.cpp $(pkg-config opencv4 --cflags --libs) -o task7

- Instructions to run the executables:
	- tasks 1, 2, 3: ./task123
		- press 's' to save a calibration frame
		- press 'c' to calibrate camera (at least 5 calibration frames are needed)
		- press 'q' to quit the program
	- tasks 4, 5, 6: ./task456
		- press 'q' to quit the program
	- tasks 7: ./task7
		- press 'q' to quit the program

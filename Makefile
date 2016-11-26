CFLAGS = -Isrc $(shell pkg-config --cflags opencv)
LDFLAGS = -lGL -lGLU -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_video -lopencv_objdetect -lopencv_imgcodecs

create_txt_anno: src/create_txt_anno.cpp TMEMotorwayProcessor.o Utilities.o
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $^

TMEMotorwayProcessor.o: src/TMEMotorwayProcessor.cpp
	$(CXX) -c -o $@ $<

Utilities.o: src/Utilities.cpp
	$(CXX) -c -o $@ $<

clean:
	rm -f *.o

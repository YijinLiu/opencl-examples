OPENCL_VERSION?=120
CFLAGS:=-g -D_POSIX_C_SOURCE=200809L -D_FILE_OFFSET_BITS=64
CXXFLAGS:=-g -O3 -pthread -std=c++11 -D_POSIX_C_SOURCE=200809L -D_FILE_OFFSET_BITS=64 \
	-DCL_TARGET_OPENCL_VERSION=$(OPENCL_VERSION)
LDFLAGS:=-lOpenCL -lstdc++ -pthread -ldl

integral_pi: integral_pi.o integral_pi_cl.o
	g++ -o $@ $^ $(LDFLAGS)

compile_cl: compile_cl.o
	g++ -o $@ $^ $(LDFLAGS)

.cc.o:
	g++ $(CXXFLAGS) -c -o $@ $<

.c.o:
	gcc $(CFLAGS) -c -o $@ $<

integral_pi.o: integral_pi_cl.h

%_cl.c %_cl.h: %.cl compile_cl
	./compile_cl $*.cl

clean:
	rm -f *.o *_cl.[ch] compile_cl integral_cl

VPATH=$(GEN_DIR)

CXX=g++
INCLUDE_DIR=
LIBS_DIR=
LIBS=-lthrift
FLAG=
GEN_DIR=../gen-cpp

#OBJ=CppClient.o CppServer.o
GEN_OBJ=Calculator.o shared_constants.o SharedService.o shared_types.o tutorial_constants.o tutorial_types.o
GEN_PATH_OBJ=$(addprefix $(GEN_DIR)/,$(GEN_OBJ))

all:CppClient CppServer

CppClient: CppClient.o $(GEN_PATH_OBJ)
	$(CXX) $(FLAG) $< $(GEN_PATH_OBJ) $(INCLUDE_DIR) -o $@ $(LIBS_DIR) $(LIBS)
	
CppServer: CppServer.o $(GEN_PATH_OBJ)
	$(CXX) $(FLAG) $< $(GEN_PATH_OBJ) $(INCLUDE_DIR) -o $@ $(LIBS_DIR) $(LIBS)

%.o: %.cpp
	$(CXX) $(FLAG) -c $< -o $@
	
clean:
	rm -f *.o $(GEN_PATH_OBJ) CppClient CppServer

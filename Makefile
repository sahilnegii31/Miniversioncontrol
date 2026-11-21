CXX = clang++
CXXFLAGS = -std=c++17 -Wall -O2

COMMON_SRCS = commit.cpp staging.cpp revert.cpp login.cpp branch.cpp shell.cpp
SERVER_SRCS = $(COMMON_SRCS) contribution.cpp c_chart.cpp server.cpp
CLI_SRCS = main.cpp $(COMMON_SRCS)

all: minivcs server

minivcs: $(CLI_SRCS)
	$(CXX) $(CXXFLAGS) $(CLI_SRCS) -o minivcs

server: $(SERVER_SRCS)
	$(CXX) $(CXXFLAGS) $(SERVER_SRCS) -o server

frontend-build:
	cd frontend && npm run build

run-server: server
	./server 8000

clean:
	rm -f minivcs server a.out

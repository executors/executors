EXAMPLES = \
  executor \
  static_thread_pool

CXXFLAGS = -std=c++14 -pthread -Wall -Wextra -I../include

.PHONY: all clean

all: $(EXAMPLES)

clean:
	rm -f $(EXAMPLES)

$(EXAMPLES): %: %.cpp
	$(CXX) $(CXXFLAGS) -o$@ $<

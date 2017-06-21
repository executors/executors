EXAMPLES = \
  executor \
  future \
  static_thread_pool

CXXFLAGS = -std=c++14 -pthread -Wall -Wextra -I../include -g

.PHONY: all clean

all: $(EXAMPLES)

clean:
	rm -f $(EXAMPLES)

$(EXAMPLES): %: %.cpp
	$(CXX) $(CXXFLAGS) -o$@ $<

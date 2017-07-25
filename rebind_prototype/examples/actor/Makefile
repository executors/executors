EXAMPLES = \
	actor_1 \
	actor_2

CXXFLAGS = -std=c++14 -pthread -Wall -Wextra -I../../include -O3

.PHONY: all clean

all: $(EXAMPLES)

clean:
	rm -f $(EXAMPLES)

$(EXAMPLES): %: %.cpp
	$(CXX) $(CXXFLAGS) -o$@ $<

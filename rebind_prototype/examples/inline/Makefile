EXAMPLES = \
	inline_1 \
	inline_2 \
	inline_3 \
	inline_4 \
	inline_5 \
	inline_6

CXXFLAGS = -std=c++14 -pthread -Wall -Wextra -I../../include

.PHONY: all clean

all: $(EXAMPLES)

clean:
	rm -f $(EXAMPLES)

$(EXAMPLES): %: %.cpp
	$(CXX) $(CXXFLAGS) -o$@ $<

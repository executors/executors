EXAMPLES = \
  adapted_and_preferable \
  adapted_and_not_preferable \
  not_adapted_and_preferable \
  not_adapted_and_not_preferable \
  priority

CXXFLAGS = -std=c++14 -pthread -Wall -Wextra -I../../include

.PHONY: all clean

all: $(EXAMPLES)

clean:
	rm -f $(EXAMPLES)

$(EXAMPLES): %: %.cpp
	$(CXX) $(CXXFLAGS) -o$@ $<

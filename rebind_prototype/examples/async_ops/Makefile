EXAMPLES = \
	async_op_1 \
	async_op_2 \
	polymorphic_async_op_1 \
	polymorphic_async_op_2

CXXFLAGS = -std=c++14 -pthread -Wall -Wextra -I../../include

.PHONY: all clean

all: $(EXAMPLES)

clean:
	rm -f $(EXAMPLES)

$(EXAMPLES): %: %.cpp
	$(CXX) $(CXXFLAGS) -o$@ $<

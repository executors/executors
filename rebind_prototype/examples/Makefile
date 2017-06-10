DIRS = \
	actor \
	adapter \
	async \
	async_ops \
	hints \
	inline \
	nested \
	simple

.PHONY: all clean

all:
	$(foreach DIR,$(DIRS),make -C $(DIR) $(MAKEFLAGS) && )true

clean:
	$(foreach DIR,$(DIRS),make -C $(DIR) $(MAKEFLAGS) clean && )true

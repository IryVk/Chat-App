.PHONY: all client server common clean

all: client server common

client:
	@echo "Making client"
	$(MAKE) -C client

server:
	@echo "Making server"
	$(MAKE) -C server

common:
	@echo "Making common"
	$(MAKE) -C common

clean:
	@echo "Cleaning up"
	$(MAKE) -C client clean
	$(MAKE) -C server clean
	$(MAKE) -C common clean
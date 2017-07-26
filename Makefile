CarlaServer:
	cd Source/CarlaServer && $(MAKE)

all:
	cd Source/CarlaServer && $(MAKE) debug
	cd Source/CarlaServer && $(MAKE) release

clean:
	cd Source/CarlaServer && $(MAKE) clean

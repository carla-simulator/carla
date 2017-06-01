CarlaServer:
	cd Source/CarlaServer && $(MAKE)

all:
	cd Source/CarlaServer && $(MAKE) debug
	cd Source/CarlaServer && $(MAKE) release
	cd Source/CarlaServer && $(MAKE) debug_png
	cd Source/CarlaServer && $(MAKE) release_png

clean:
	cd Source/CarlaServer && $(MAKE) clean

clean-all:
	cd Source/CarlaServer && $(MAKE) clean-all

CarlaServer:
	cd Source/CarlaServer && $(MAKE)

clean:
	cd Source/CarlaServer && $(MAKE) clean-all
	rm -Rf Binaries Intermediate

How to convert the PHEMlight C# code to C++
===========================================

- Use C# to C++ converter by Tangible Software
- load the ConverterOptions.dat via Import Settings
- Select "Convert all C# in folder"
- Select the dll_code folder as source and the cpp folder as target
- determine changes in Start.cpp (function PHEMLight::CreateVehicleStateData) which need to go into HelpersPHEMlight::compute
- delete cResult.*, stringbuilder.h, and Start.*
- apply the conversion patch to add license info and some small fixes
- check whether further changes are needed by compiling and running the tests, especially the reference cycles
- if adaptions were made create a new conversion patch (git diff *.cpp *.h > conversion.patch)
- commit

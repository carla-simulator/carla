# SimReady content with NVIDIA Omnivers

NVIDIA's [__SimReady__](#simready-content-from-nvidias-omniverse) specification supports the preparation of 3D content that is purpose-built for simulation to help streamline content creation pipelines for simulating 3D virtual environments for machine learning purposes in robotics and autonomous driving. Through the Omniverse Unreal Engine plugin now integrated into CARLA users can import, in just a few clicks, SimReady content such as vehicles already configured with working lights, doors and wheels and props ready for use instantaneously to decorate CARLA maps. CARLA's Omniverse integration boasts to significantly accelerate your environment building pipeline and opens the door to a whole world of applications in the [__Omniverse ecosystem__](https://www.nvidia.com/en-us/omniverse/ecosystem/). 

The Omniverse Unreal Engine Connector is currently only available in Windows.

Follow these stesps to start using Omniverse and using SimReady content in CARLA:

Before anything else, you should first [install NVIDIA Omniverse](https://docs.omniverse.nvidia.com/install-guide/latest/index.html)

### 1. Installing the Unreal Engine Omniverse Connector

1. Launch the NVIDIA Omniverse Launcher.
2. Navigate to the Exchange tab.
3. Locate the Epic Games Unreal Engine 4.26 Omniverse Connector.
4. Ensure the Install version is Release 105.1.578
5. Click *Install*.
6. Omniverse will not find Unreal Engine, click OK.
7. It will prompt you to find your Unreal Engine installation, Select this path: {UE4_ROOT}\Engine\Plugins\Marketplace\NVIDIA
8. Press *Install*.

### 2. Set Up Local Omniverse Server

1. Launch the NVIDIA Omniverse Launcher.
2. Navigate to the Nucleus tab.
3. Click on create Local Server.
4. Add administrator details (any).
5. Select on the folder icon next to Local Nucleus Service, this should open your localhost server navigator.

### 3. Connecting to CARLA Simulator

1. Launch CARLA using `make launch` from the command line ine the CARLA root folder.
2. If an existing server is active, and you want to restart the setup, click on Clean Local Assets. (Optional)
3. Select the Omniverse icon and click on *Add Server*.
4. Name the server and click *Add to Content Browser*.
5. A login form should launch in your browser, click *Create Account*.
6. Add administrator details(any).
7. Omniverse folders should have appeared in your Content/Omniverse folder within the Engine.
8. Go to your local host browser Navigator
9. Click on *Connect to a Server*.
10. Authenticate using the server name you set for the CARLA server.
11. Use administrator details you set for the CARLA server.
12. The server folders should now show up in your browser navigator.

### 4. Importing the SimReady Car Asset

1. Navigate to the Projects folder within the browser navigator.
2. Right click, and select Upload Folder.
3. Select your SimReady folders.
4. Upload the files.

### 5. Using the Omniverse Connector to Load the Vehicle to CARLA

1. Open the CARLA project in Unreal Engine.
2. Navigate to CarlaTools Content\USDImporter.
3. Right click on *UW_USDVehicleImporterEditorWidget*.
4. Select *Run Editor Utility Widget*.
5. Find the Vehicle within your Omniverse browser navigator
6. Copy the path (should look something like: omniverse://localhost/Projects/SimReadyUSD.../vehicle.usd)
7. Paste it inside the Omiverse URL tab within the Widget.
8. Select Import Asset.
9. You should see the vehicle show up in the engine, within Content/OmniverseImports.
10. Open another map scene since it opened a new scene with the imported data, a *Save Content* prompt should appear. Uncheck the *Untitled* scene, and click on *Save selected* to save new content.
11. The vehicle is now usable within CARLA.

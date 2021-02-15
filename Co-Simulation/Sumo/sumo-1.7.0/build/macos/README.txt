These application bundles are necessary wrappers for the SUMO binaries in order to have a native feel on macOS. They have been built with Platypus [1]. Platypus is a developer tool that creates native Mac applications from command line scripts such as shell scripts or Python, Perl, Ruby, Tcl, JavaScript and PHP programs. 
In order to rebuild these bundles, the icon files and the launcher scripts are supplied in the build subfolder. The following steps are necessary to create the bundle:

1) Download and install platypus
2) Create a new bundle by opening platypus
3) Select the launcher script in the build subfolders of the SUMO repository
4) Give the bundle a proper name (sumo-gui, netedit, OSM Web Wizard)
5) Change the default icon to one of the icon files in the build subfolder
6) Change the "Interface" type to "None"
7) Ensure that org.eclipse.sumo.<APPNAME> is selected as Identifier
8) Ensure that "Eclipse SUMO" is stated as Author
9) Ensure that Version matches the SUMO version
10) Disable the "Accept dropped items" checkbox
11) Disable the "Run with root privileges" checkbox
12) Disable the "Run in background" checkbox
13) Disable the "Remain running after execution" checkbox
14) Create the app bundle by clicking on "Create App"
15) Select the folder to store the app
16) Optimization may be turned on by enabling the checkbox
17) Disable the creation of a symlinked version by disabling the checkbox

The launcher scripts try to find the SUMO_HOME variable setting by sourcing several configuration files (.profile, .bash_profile, .zshrc). If the script was unable to find a SUMO_HOME declaration a warning is issued. You may extend the list of configuration files, if you defined the SUMO_HOME in a different file.

[1]: https://sveinbjorn.org/platypus

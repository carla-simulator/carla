/*
 * This is an example for a POVRay include file that can be used to configure the rendering
 * of .pov files generated with OSM2World (http://osm2world.org/).
 *
 * You can leave out any definition from this file, the rendering will then fall back
 * to (mostly very basic) default materials and shapes.
 * Please be aware that many possible definitions are not listed in this example file.
 * Examine the "material and object definitions" section of a .pov file generated
 * by OSM2World to see what definitions are used by that particular scene.
 */

#include "metals.inc"
#include "woods.inc"
#include "textures.inc"
#include "colors.inc"

// chooses the season - possible values: spring, summer, autumn, winter
// (has no effect on the generated scenes yet, could be used in custom model declarations)
#declare season="summer"

// chooses the time of day - possible values: day, night
// (has no effect on the generated scenes yet, could be used in custom model declarations)
#declare time="day"

// activate this #include if you have downloaded the tree models
// that are available from http://osm2world.org/download (optional)
// #include "osm2world_trees.inc"

// texture definitions

#declare texture_ASPHALT=texture { pigment { color rgb <0.2, 0.2, 0.2> } }
#declare texture_WATER=texture { T_Chrome_2D normal { waves 0.2 frequency 1000.0 scale 4.0 turbulence 0.1 rotate <0,-45,0> } }
#declare texture_WOOD=texture { T_Wood5 }
#declare texture_TERRAIN_DEFAULT=texture { pigment { color rgb <0.2, 0.73, 0.2> } finish { ambient 0.5 diffuse 0.5 } }

// sky sphere definition

// activating this creates a pure blue sky.
// This is useful when generating tiles. With patterned skies,
// there will often be visible tile borders in reflecting water.
//#declare sky_sphere_def = sky_sphere { pigment { rgb <0.0, 0.0, 1.0> } }


// activating this creates reflective windows
// However, you need a material map which is a indexed png with 3 colors:
// Color 1 is for the transparent part, color 2 is for the window glass, color 3 
// is for the rest of the window, i.e. the window frame 
//#declare texture_BUILDING_WINDOWS = texture { 
//  material_map {
//    png "./textures/transparent.png"
//    texture { pigment { color rgb<0,0,0> transmit 1 } }
//    texture {
//      pigment { image_map {
//        png "./textures/MarekCompositeWall00001_transparent.png"
//      } }
//      finish {
//        ambient 0.5
//        diffuse 0.5
//        phong 1.0
//        phong_size 150
//        reflection 0.5
//      }
//    }
//    texture {
//      pigment { image_map {
//        png "./textures/MarekCompositeWall00001_transparent.png"
//      } }
//      finish {
//        ambient 0.5
//        diffuse 0.5
//      }
//    }
//  } }

//wrap the file with the version
#local Temp_version = version;
#version 3.7;
//==================================================
//POV-Ray Materials list
//==================================================
//This file has been created by PoseRay v3.13.29.645
//3D model to POV-Ray/Moray Converter.
//Author: FlyerX
//Email: flyerx_2000@yahoo.com
//Web: https://sites.google.com/site/poseray/
//==================================================
//Files needed to run the POV-Ray scene:
//sedan_rim_POV_main.ini (initialization file - open this to render)
//sedan_rim_POV_scene.pov (scene setup of cameras, lights and geometry)
//sedan_rim.inc (geometry)
//sedan_rim_mat.inc (materials)
// 
 
//Macros used by PoseRay to manipulate pigments
//October 06, 2010
//By FlyerX 

#macro pigment_fade(p1,f)
//filters a pigment by a value f. f=0 pigment is white, f=1 pigment is p1
#local PR1 = function {pigment{p1}}
#local PR_FRed=function (x,y,z) {(PR1(x,y,z).red-1)*f+1}
#local PR_FGrn=function (x,y,z) {(PR1(x,y,z).green-1)*f+1}
#local PR_FBlu=function (x,y,z) {(PR1(x,y,z).blue-1)*f+1}
average pigment_map{
    [function{PR_FRed(x,y,z)} color_map{[0 rgb 0][1 rgb <1,0,0>*3]}]
    [function{PR_FGrn(x,y,z)} color_map{[0 rgb 0][1 rgb <0,1,0>*3]}]
    [function{PR_FBlu(x,y,z)} color_map{[0 rgb 0][1 rgb <0,0,1>*3]}]
  }
#end 

#macro pigment_gray(p1)
//converts to grayscale using luminance
#local PR1 = function {pigment{p1}}
#local PR_Gray=function (x,y,z) {0.2126*PR1(x,y,z).red+0.7152*PR1(x,y,z).green+0.0722*PR1(x,y,z).blue}
function{PR_Gray(x,y,z)}
#end  

#macro pigment_multiply(p1,p2)
//multiplies two pigments p1xp2
#local PR1 = function {pigment{p1}}
#local PR2 = function {pigment{p2}}
#local PR_FRed=function (x,y,z) {PR1(x,y,z).red*PR2(x,y,z).red}
#local PR_FGrn=function (x,y,z) {PR1(x,y,z).green*PR2(x,y,z).green}
#local PR_FBlu=function (x,y,z) {PR1(x,y,z).blue*PR2(x,y,z).blue} 
average pigment_map{
    [function{PR_FRed(x,y,z)} color_map{[0 rgb 0][1 rgb <1,0,0>*3]}]  //add each color and then divide by 3
    [function{PR_FGrn(x,y,z)} color_map{[0 rgb 0][1 rgb <0,1,0>*3]}]
    [function{PR_FBlu(x,y,z)} color_map{[0 rgb 0][1 rgb <0,0,1>*3]}]    
  }
#end 

#macro pigment_screen(p1,p2)
//screens two pigments p1 p2
#local PR1 = function {pigment{p1}}
#local PR2 = function {pigment{p2}}
#local PR_FRed=function (x,y,z) {1-(1-PR1(x,y,z).red)*(1-PR2(x,y,z).red)}
#local PR_FGrn=function (x,y,z) {1-(1-PR1(x,y,z).green)*(1-PR2(x,y,z).green)}
#local PR_FBlu=function (x,y,z) {1-(1-PR1(x,y,z).blue)*(1-PR2(x,y,z).blue)} 
average pigment_map{
    [function{PR_FRed(x,y,z)} color_map{[0 rgb 0][1 rgb <1,0,0>*3]}]  //add each color and then divide by 3
    [function{PR_FGrn(x,y,z)} color_map{[0 rgb 0][1 rgb <0,1,0>*3]}]
    [function{PR_FBlu(x,y,z)} color_map{[0 rgb 0][1 rgb <0,0,1>*3]}]    
  }
#end        

#macro pigment_divide(p1,p2)
//divides pigment p2 by p1
//1/256 is added to the dividing color to prevent division by zero 
//colors are clipped to 0-1
#local PR1 = function {pigment{p1}}
#local PR2 = function {pigment{p2}}
#local PR_FRed=function (x,y,z) {clip(PR2(x,y,z).red/(PR1(x,y,z).red+0.004),0,1)}
#local PR_FGrn=function (x,y,z) {clip(PR2(x,y,z).green/(PR1(x,y,z).green+0.004),0,1)}
#local PR_FBlu=function (x,y,z) {clip(PR2(x,y,z).blue/(PR1(x,y,z).blue+0.004),0,1)} 
average pigment_map{
    [function{PR_FRed(x,y,z)} color_map{[0 rgb 0][1 rgb <1,0,0>*3]}]  //add each color and then divide by 3
    [function{PR_FGrn(x,y,z)} color_map{[0 rgb 0][1 rgb <0,1,0>*3]}]
    [function{PR_FBlu(x,y,z)} color_map{[0 rgb 0][1 rgb <0,0,1>*3]}]    
  }
#end  


#macro pigment_add(p1,p2)
//adds two pigments - values are clipped 0 to 1
#local PR1 = function {pigment{p1}}
#local PR2 = function {pigment{p2}}
#local PR_FRed=function (x,y,z) {clip(PR1(x,y,z).red+PR2(x,y,z).red,0,1)}
#local PR_FGrn=function (x,y,z) {clip(PR1(x,y,z).green+PR2(x,y,z).green,0,1)}
#local PR_FBlu=function (x,y,z) {clip(PR1(x,y,z).blue+PR2(x,y,z).blue,0,1)}
average pigment_map{
    [function{PR_FRed(x,y,z)} color_map{[0 rgb 0][1 rgb <1,0,0>*3]}]
    [function{PR_FGrn(x,y,z)} color_map{[0 rgb 0][1 rgb <0,1,0>*3]}]
    [function{PR_FBlu(x,y,z)} color_map{[0 rgb 0][1 rgb <0,0,1>*3]}]
  }
#end 

#macro pigment_substract(p1,p2)
//subtracts two pigments - values are clipped 0 to 1
#local PR1 = function {pigment{p1}}
#local PR2 = function {pigment{p2}}
#local PR_FRed=function (x,y,z) {clip(PR2(x,y,z).red-PR1(x,y,z).red,0,1)}
#local PR_FGrn=function (x,y,z) {clip(PR2(x,y,z).green-PR1(x,y,z).green,0,1)}
#local PR_FBlu=function (x,y,z) {clip(PR2(x,y,z).blue-PR1(x,y,z).blue,0,1)}
average pigment_map{
    [function{PR_FRed(x,y,z)} color_map{[0 rgb 0][1 rgb <1,0,0>*3]}]
    [function{PR_FGrn(x,y,z)} color_map{[0 rgb 0][1 rgb <0,1,0>*3]}]
    [function{PR_FBlu(x,y,z)} color_map{[0 rgb 0][1 rgb <0,0,1>*3]}]
  }
#end

#macro pigment_colorburn(p1,p2)
//burns p1 onto p2
//if image maps, they should be set to gamma 1 or it will give wrong results. 
//resulting image should be gamma corrected again using pigment_power(pigment,2.2)
#local PR1 = function {pigment{p1}}
#local PR2 = function {pigment{p2}}
#local PR_FRed=function (x,y,z) {clip(1-((1-PR2(x,y,z).red)/(PR1(x,y,z).red+0.004)),0,1)}
#local PR_FGrn=function (x,y,z) {clip(1-((1-PR2(x,y,z).green)/(PR1(x,y,z).green+0.004)),0,1)}
#local PR_FBlu=function (x,y,z) {clip(1-((1-PR2(x,y,z).blue)/(PR1(x,y,z).blue+0.004)),0,1)} 
average pigment_map{
    [function{PR_FRed(x,y,z)} color_map{[0 rgb 0][1 rgb <1,0,0>*3]}]  //add each color and then divide by 3
    [function{PR_FGrn(x,y,z)} color_map{[0 rgb 0][1 rgb <0,1,0>*3]}]
    [function{PR_FBlu(x,y,z)} color_map{[0 rgb 0][1 rgb <0,0,1>*3]}]    
  }
#end 

#macro pigment_linearburn(p1,p2)
//burns p1 onto p2
#local PR1 = function {pigment{p1}}
#local PR2 = function {pigment{p2}}
#local PR_FRed=function (x,y,z) {clip(PR2(x,y,z).red+PR1(x,y,z).red-1,0,1)}
#local PR_FGrn=function (x,y,z) {clip(PR2(x,y,z).green+PR1(x,y,z).green-1,0,1)}
#local PR_FBlu=function (x,y,z) {clip(PR2(x,y,z).blue+PR1(x,y,z).blue-1,0,1)} 
average pigment_map{
    [function{PR_FRed(x,y,z)} color_map{[0 rgb 0][1 rgb <1,0,0>*3]}]  //add each color and then divide by 3
    [function{PR_FGrn(x,y,z)} color_map{[0 rgb 0][1 rgb <0,1,0>*3]}]
    [function{PR_FBlu(x,y,z)} color_map{[0 rgb 0][1 rgb <0,0,1>*3]}]    
  }
#end 

#macro pigment_overlay(p1,p2)
//overlays p1 onto p2
//if image maps, they should be set to gamma 1 or it will give wrong results. 
//resulting image should be gamma corrected again using pigment_power(pigment,2.2)
#local PR1 = function {pigment{p1}}
#local PR2 = function {pigment{p2}}


#local PR_FRed=function (x,y,z) {select((PR2(x,y,z).red-0.5),
clip(2*PR1(x,y,z).red*PR2(x,y,z).red,0,1),
clip(1-2*(1-PR1(x,y,z).red)*(1-PR2(x,y,z).red),0,1))}

#local PR_FGrn=function (x,y,z) {select((PR2(x,y,z).green-0.5),
clip(2*PR1(x,y,z).green*PR2(x,y,z).green,0,1),
clip(1-2*(1-PR1(x,y,z).green)*(1-PR2(x,y,z).green),0,1))}

#local PR_FBlu=function (x,y,z) {select((PR2(x,y,z).blue-0.5),
clip(2*PR1(x,y,z).blue*PR2(x,y,z).blue,0,1),
clip(1-2*(1-PR1(x,y,z).blue)*(1-PR2(x,y,z).blue),0,1))}

average pigment_map{
    [function{PR_FRed(x,y,z)} color_map{[0 rgb 0][1 rgb <1,0,0>*3]}]  //add each color and then divide by 3
    [function{PR_FGrn(x,y,z)} color_map{[0 rgb 0][1 rgb <0,1,0>*3]}]
    [function{PR_FBlu(x,y,z)} color_map{[0 rgb 0][1 rgb <0,0,1>*3]}]    
  }
#end 

#macro pigment_transmit(p1,tr)
//sets the transparency of p1 to tr
#local PR1 = function {pigment{p1}} 
#local PR_FRed=function (x,y,z) {PR1(x,y,z).red}
#local PR_FGrn=function (x,y,z) {PR1(x,y,z).green}
#local PR_FBlu=function (x,y,z) {PR1(x,y,z).blue} 
average pigment_map{
    [function{PR_FRed(x,y,z)} color_map{[0 rgbft <0,0,0,0,tr>][1 rgbft <1,0,0,0,tr/3>*3]}]
    [function{PR_FGrn(x,y,z)} color_map{[0 rgbft <0,0,0,0,tr>][1 rgbft <0,1,0,0,tr/3>*3]}]
    [function{PR_FBlu(x,y,z)} color_map{[0 rgbft <0,0,0,0,tr>][1 rgbft <0,0,1,0,tr/3>*3]}]   
  }
#end  

#macro pigment_process(p1,p2,tr)
//multiplies two pigments and sets the transparency to tr
#local PR1 = function {pigment{p1}}
#local PR2 = function {pigment{p2}}
#local PR_FRed=function (x,y,z) {PR1(x,y,z).red*PR2(x,y,z).red}
#local PR_FGrn=function (x,y,z) {PR1(x,y,z).green*PR2(x,y,z).green}
#local PR_FBlu=function (x,y,z) {PR1(x,y,z).blue*PR2(x,y,z).blue} 
average pigment_map{
    [function{PR_FRed(x,y,z)} color_map{[0 rgbft <0,0,0,0,tr>][1 rgbft <1,0,0,0,tr/3>*3]}]
    [function{PR_FGrn(x,y,z)} color_map{[0 rgbft <0,0,0,0,tr>][1 rgbft <0,1,0,0,tr/3>*3]}]
    [function{PR_FBlu(x,y,z)} color_map{[0 rgbft <0,0,0,0,tr>][1 rgbft <0,0,1,0,tr/3>*3]}]    
  }
#end   



#macro pigment_power(p1,n)
//raises a pigment to a power - works with images with no mask (jpeg, bmp...)
#local PR1 = function {pigment{p1}}
#local PR_FRed=function (x,y,z) {pow(PR1(x,y,z).red,n)}
#local PR_FGrn=function (x,y,z) {pow(PR1(x,y,z).green,n)}
#local PR_FBlu=function (x,y,z) {pow(PR1(x,y,z).blue,n)}
average pigment_map{
    [function{PR_FRed(x,y,z)} color_map{[0 rgb 0][1 rgb <1,0,0>*3]}]
    [function{PR_FGrn(x,y,z)} color_map{[0 rgb 0][1 rgb <0,1,0>*3]}]
    [function{PR_FBlu(x,y,z)} color_map{[0 rgb 0][1 rgb <0,0,1>*3]}]
  }
#end 


#macro pigment_threshold(p1,s)
//makes a pigment black if color < s or white otherwise
#local PR1 = function {pigment{p1}}
#local PR_FRed=function (x,y,z) {0.5*(sgn(PR1(x,y,z).red-s)+1)}
#local PR_FGrn=function (x,y,z) {0.5*(sgn(PR1(x,y,z).green-s)+1)}
#local PR_FBlu=function (x,y,z) {0.5*(sgn(PR1(x,y,z).blue-s)+1)}
average pigment_map{
    [function{PR_FRed(x,y,z)} color_map{[0 rgb 0][1 rgb <1,0,0>*3]}]
    [function{PR_FGrn(x,y,z)} color_map{[0 rgb 0][1 rgb <0,1,0>*3]}]
    [function{PR_FBlu(x,y,z)} color_map{[0 rgb 0][1 rgb <0,0,1>*3]}]
  }
#end  


#macro pigment_invert(p1)
//inverts the color -> 1-RGB
#local PR1 = function {pigment{p1}}
#local PR_FRed=function (x,y,z) {1-PR1(x,y,z).red}
#local PR_FGrn=function (x,y,z) {1-PR1(x,y,z).green}
#local PR_FBlu=function (x,y,z) {1-PR1(x,y,z).blue}
average pigment_map{
    [function{PR_FRed(x,y,z)} color_map{[0 rgb 0][1 rgb <1,0,0>*3]}]
    [function{PR_FGrn(x,y,z)} color_map{[0 rgb 0][1 rgb <0,1,0>*3]}]
    [function{PR_FBlu(x,y,z)} color_map{[0 rgb 0][1 rgb <0,0,1>*3]}]
  }
#end 

 

#macro pigment_mix(p1,p2,xv)
//mixes two pigments on a sliding scale xv from 0 to 1 
#local PR1 = function {pigment{p1}}
#local PR2 = function {pigment{p2}}
#local PR_FRed=function (x,y,z) {PR1(x,y,z).red*(1-xv)+PR2(x,y,z).red*xv}
#local PR_FGrn=function (x,y,z) {PR1(x,y,z).green*(1-xv)+PR2(x,y,z).green*xv}
#local PR_FBlu=function (x,y,z) {PR1(x,y,z).blue*(1-xv)+PR2(x,y,z).blue*xv}
average pigment_map{
    [function{PR_FRed(x,y,z)} color_map{[0 rgb 0][1 rgb <1,0,0>*3]}]
    [function{PR_FGrn(x,y,z)} color_map{[0 rgb 0][1 rgb <0,1,0>*3]}]
    [function{PR_FBlu(x,y,z)} color_map{[0 rgb 0][1 rgb <0,0,1>*3]}]
  }
#end

//=======================================

#macro power_ramp(xv,xa,xb,ya,yb,pw)
//generates a float with a power ramp from (xa,ya) to (xb,yb) with power pw and argument xv. 
//ramp slope is zero at xb
  ((ya-yb)/pow((xa-xb),pw))*pow((xv-xb),pw)+yb
#end

#macro power_ramp2(xv,xa,xb,ya,yb,pw)
//generates a float with a power ramp from (xa,ya) to (xb,yb) with power pw and argument xv. 
//ramp slope is zero at xa
  ((yb-ya)/pow((xb-xa),pw))*pow((xv-xa),pw)+ya
#end

#macro linear_ramp(xv,xa,xb,ya,yb)
//generates a linear float from (xa,ya) to (xb,yb)
  (yb-ya)/(xb-xa)*(xv-xa)+ya
#end
 
//==================================================
//Map definitions
 
//==================================================
//Material definitions
#declare Material_001_=
#declare PR_DIFFUSE=pigment {color srgb <0.19,0.19,0.19> transmit 0/100 }


#declare F1=finish{specular  0.709803921568627 //Max highlight intensity: 0 -> 1
                   roughness 0.0076559483058079 //surface roughness: 0.0005 -> 1
                   phong  0 phong_size 0 //Phong specular disabled
                   
                   ambient <0,0,0> emission srgb <1.00,1.00,1.00> //ambient RGB color: 0->1 for each component
                   diffuse  0.6 
                   reflection{0 } conserve_energy //reflection amount: 0 -> 1
                   }


#declare T1=texture{
                   pigment{uv_mapping PR_DIFFUSE }
                            finish{F1}
                    }


material{
         texture{/*uv_mapping*/
                 T1
                }
}
 
//restore the version used outside this file
#version Temp_version;
//==================================================

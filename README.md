# RmSAT-CFAR (Rayleigh Mixture Summed Area Tables-Constant False Alarm Rate)

This repository holds source code for Rayleigh Mixture Summed Area Tables-Constant False Alarm Rate (RmSAT-CFAR).
Constant False Alarm Rate (CFAR) is the most used framework for target detection in Synthetic Aperture Radar (SAR) images.
Using a Rayleigh Mixture (RM) model, background statistics are modeled.
Summed Area Tables (SAT) are used to improve detection speed.
Parallel implementation of image tiles is used for fast computation. 

## How to use

You can compile code using provided Visual Studio 2015 project or use already compiled code.

# Executables

All already compiled executables for windows can be found in the [releases](https://github.com/ati-ozgur/RmSAT-CFAR/releases).
Or you can download [latest executable](https://github.com/ati-ozgur/RmSAT-CFAR/releases/latest).

First, you need to install Visual C++ Redistributable for Visual Studio 2015.
Either use vc_redist.x64.exe in downloaded zip file or [download it](https://www.microsoft.com/en-us/download/details.aspx?id=48145) from Microsoft.




# Compilation

## For developer computer following configuration is suggested.

- Intel i7  (at least 4-core and 3.0 Ghz)
- 16 GB RAM (suggested 32 GB)


## Necessary Software  

- [Visual Studio 2015](https://www.visualstudio.com/vs/visual-studio-express/)  C++ IDE
- [OpenCV 3.1 64bit](http://opencv.org/) (image processing library)


You should download suitable version of OpenCV that is already compiled for Visual Studio.
For example Visual Studio 2015 C++ is the version vc14.
OpenCV 3.1 already provides already compiled DLL and LIB files for this version.

 - \OpenCV\build\x64\vc14\bin    
 - \OpenCV\build\x64\vc14\lib


## Environment Variables

To make it easy to compile source code using different open cv installations, project file for Visual Studio 2015 uses following environment variables with given default values.

- VisualCppVersion=vc140
- OpenCVDirectory=D:\OpenCV\
- OpenCVVersion=310

You may choose to set this environment variables using windows standard mechanism for [setting environment variables](https://www.java.com/en/download/help/path.xml).
Or you may use provided bat file [vs2015.bat](https://github.com/ati-ozgur/RmSAT-CFAR/blob/master/VS2015.bat).
You should change VS2015.bat so that these environment variables point to correct paths.


After these steps, you should be able to compile project using Visual Studio 2015.


# Example run.

     CFARtargetDetection.exe im1024.tif im2014-target.png RmSAT-CFAR 0.0001

# If you run CFARtargetDetection.exe without arguments, help information can be seen.






# Problem

The program can't start because MSVCP140.dll is missing from your computer. Try reinstalling the program to fix this problem. 


## Solution

Install Visual C++ Redistributable for Visual Studio 2015.
Either use vc_redist.x64.exe in executable zip or [download it](https://www.microsoft.com/en-us/download/details.aspx?id=48145) from Microsoft.

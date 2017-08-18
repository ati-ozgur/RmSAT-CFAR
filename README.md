# RmSAT-CFAR (Rayleigh Mixture Summed Area Tables-Constant False Alarm Rate)

This repository holds source code for Rayleigh Mixture Summed Area Tables-Constant False Alarm Rate (RmSAT-CFAR).
Constant False Alarm Rate (CFAR) is the most used framework for target detection in Synthetic Aperture Radar (SAR) images.
Using a Rayleigh Mixture (RM) model, background statistics are modeled.
Summed Area Tables (SAT) are used to improve detection speed.
Parallel implementation of image tiles is used for fast computation. 

## How to use

You can compile code using provided Visual Studio 2015 project or use already compiled code.

### Executables

All already compiled executables for windows can be found in the [releases](https://github.com/ati-ozgur/RmSAT-CFAR/releases).
Or you can download [latest executable](https://github.com/ati-ozgur/RmSAT-CFAR/releases/latest).



## Compilation

### For developer computer following configuration is suggested.

- Intel i7  (at least 4-core and 3.0 Ghz)
- 16 GB RAM (suggested 32 GB)


### Necessary Software  

- [Visual Studio 2015](https://www.visualstudio.com/vs/visual-studio-express/)  C++ IDE
- [OpenCV 3.1 64bit](http://opencv.org/) (image processing library)







# Example run.

     CFARtargetDetection.exe im1024.tif im2014-target.png RmSAT-CFAR 0.0001

# If you run CFARtargetDetection.exe without arguments, help information can be seen.


# RmSAT-CFAR (Rayleigh Mixture Summed Area Tables-Constant False Alarm Rate)

This repository holds source code for Rayleigh Mixture Summed Area Tables-Constant False Alarm Rate (RmSAT-CFAR).
Constant False Alarm Rate (CFAR) is the most used framework for target detection in Synthetic Aperture Radar (SAR) images.
Using a Rayleigh Mixture (RM) model, background statistics are modeled.
Summed Area Tables (SAT) are used to improve detection speed.
Parallel implementation of image Tiles is used for fast computation. 

## How to use
Compiled executable for windows is provided in the [releases](https://github.com/ati-ozgur/RmSAT-CFAR/releases).


# Download executables from Release Tab.

# Example run.

     CFARtargetDetection.exe im1024.tif im2014-target.png RmSAT-CFAR 0.0001

# If you run CFARtargetDetection.exe without arguments, help information can be seen.


# RmSAT-CFAR: Rayleigh Mixture Summed Area Tables-Constant False Alarm Rate

This repository holds source code for Rayleigh Mixture Summed Area Tables-Constant False Alarm Rate (RmSAT-CFAR).
Constant False Alarm Rate (CFAR) is the most used framework for target detection in Synthetic Aperture Radar (SAR) images.
RmSAT-CFAR is an extension of clasical CFAR framework by modeling the background statistics using a Rayleigh Mixture (RM) model and adopting
Summed Area Tables (SAT) to improve detection speed.
Parallel implementation of image tiles is used for fast computation. 

## Adaptive Simulated Annealing (ASA)

For finding parameters of Rayleigh Mixture, Adaptive Simulated Annealing (ASA) is used.
Our implementation for ASA can be found in following [file](https://github.com/ati-ozgur/RmSAT-CFAR/blob/master/AdaptiveSimulatedAnnealing.h).
To test ASA, following non linear cost functions are also [implemented](https://github.com/ati-ozgur/RmSAT-CFAR/blob/master/NonlinearTestCostFunctions.h).
Information about these functions can be found in [here](https://en.wikipedia.org/wiki/Test_functions_for_optimization).

    - RastriginFunction
    - AckleysFunction
    - SphereFunction
    - RosenbrockFunction
    - BealesFunction
    - GoldsteinPriceFunction
    - BoothsFunction
    - BukinFunctionNo6
    - MatyasFunction
    - LeviFunctionNo13
    - ThreeHumpCamelFunction
    - EasomFunction
    - CrossInTrayFunction
    - EggholderFunction
    - HolderTableFunction
    - McCormickFunction
    - SchafferFunctionNo2
    - SchafferFunctionNo4
    - StyblinskiTangFunction






## Other implemented Algorithms

To compare RmSAT-CFAR algorithm to other CFAR algorithms, following algorithms are also implemented in the repository.
Using abstract classes provided (
 [AbstractCFAR](https://github.com/ati-ozgur/RmSAT-CFAR/blob/master/targetDetectors/AbstractCFAR.h) and [WindowBasedCFAR](https://github.com/ati-ozgur/RmSAT-CFAR/blob/master/targetDetectors/WindowBasedCFAR.h)
), other CFAR algorithms can be easily added.

1. Cell Averaging CFAR (CA-CFAR)

    Novak, L. M.; Owirka, G. J.; Brower, W. S. & Weaver, A. L.
    The automatic target-recognition system in SAIP 
    Lincoln Laboratory Journal, LINCOLN LABORATORY MIT, 1997 , 10

2. Automatic Censored CFAR (AC-CFAR)

    Farrouki, A. & Barkat, M.
    Automatic censoring CFAR detector based on ordered data variability for nonhomogeneous environments 
    IEE Proceedings-Radar, Sonar and Navigation, IET, 2005 , 152 , 43-51

3. Adaptive and Fast CFAR ([AAF-CFAR](https://github.com/ati-ozgur/RmSAT-CFAR/blob/master/targetDetectors/AdaptiveAndFastCFAR.h))


    Gao, G.; Liu, L.; Zhao, L.; Shi, G. & Kuang, G.
    An adaptive and fast CFAR algorithm based on automatic censoring for target detection in high-resolution SAR images 
    IEEE transactions on geoscience and remote sensing, IEEE, 2009 , 47 , 1685-1697

# How to use

You can compile code using provided Visual Studio 2015 project or use already compiled binaries.

## Executables

All already compiled executables for windows can be found in the [releases](https://github.com/ati-ozgur/RmSAT-CFAR/releases).
Or you can download [latest executable](https://github.com/ati-ozgur/RmSAT-CFAR/releases/latest).

First, you need to install Visual C++ Redistributable for Visual Studio 2015.
Either use vc_redist.x64.exe in downloaded zip file or [download it](https://www.microsoft.com/en-us/download/details.aspx?id=48145) from Microsoft.




## Compilation


## Required Packages 

- [Visual Studio 2015 Express](https://www.visualstudio.com/vs/visual-studio-express/)  C++ IDE
- [OpenCV 3.1 64bit](http://opencv.org/) (image processing library)


You should download suitable version of OpenCV that is already compiled for Visual Studio.
For example Visual Studio 2015 C++ is the version vc14.
OpenCV 3.1 provides already compiled DLL and LIB files for this version.

 - \OpenCV\build\x64\vc14\bin    
 - \OpenCV\build\x64\vc14\lib


## Environment Variables

To make it easy to compile source code using different OpenCV installations, project file for Visual Studio 2015 uses following environment variables with given default values.

- VisualCppVersion=vc140
- OpenCVDirectory=D:\OpenCV\
- OpenCVVersion=310

You may choose to set this environment variables using windows standard mechanism for [setting environment variables](https://www.java.com/en/download/help/path.xml).
Or you may use provided bat file [vs2015.bat](https://github.com/ati-ozgur/RmSAT-CFAR/blob/master/VS2015.bat).
You should change VS2015.bat so that these environment variables point to correct paths.


After these steps, double click VS2015.bat and open Visual Studio.
You should be able to compile project using Visual Studio 2015.



# Example Usage

If you run CFARtargetDetection.exe without arguments, help information can be seen.


```cmd
CFARtargetDetector v1.0
CFARtargetDetector  [Input File Name] [Output File Name] [Target Detection Method] [Probability Of False Alarm] [Key1] [Value1] ... [KeyN] [ValueN]
 Example : CFARtargetDetector  im1024.tif im1024_targets.png RmSAT-CFAR 1e-5 ThreadCount 1 RmSAT-CFAR.guardRadius 10 RmSAT-CFAR.maximumMixtureCount 6

RmSAT-CFAR parameters
---------------------
RmSAT-CFAR.guardRadius
RmSAT-CFAR.clutterRadius
RmSAT-CFAR.minimumMixtureCount
RmSAT-CFAR.maximumMixtureCount

AAF-CFAR parameters
-------------------
AAF-CFAR.guardRadius
AAF-CFAR.clutterRadius
AAF-CFAR.censoringPercentile

CA-CFAR, AC-CFAR, VI-CFAR parameters
------------------------------------
WB-CFAR.targetRadius
WB-CFAR.guardRadius
WB-CFAR.clutterRadius

AC-CFAR parameters
------------------
AC-CFAR.censoringPercentile

```

# Example runs.

    CFARtargetDetection.exe im1024.tif output-targets-RmSAT-CFAR.png RmSAT-CFAR 1e-5

    CFARtargetDetection.exe im1024.tif output-targets-AAFSAT-CFAR.png AAF-CFAR 1e-5

    CFARtargetDetection.exe im1024.tif output-targets-CA-CFAR.png CA-CFAR 1e-5

    CFARtargetDetection.exe im1024.tif output-targets-AC-CFAR.png AC-CFAR 1e-5

    CFARtargetDetection.exe im1024.tif output-targets-VI-CFAR.png VI-CFAR 1e-5

 



# Problem

The program can't start because MSVCP140.dll is missing from your computer. Try reinstalling the program to fix this problem. 


## Solution

Install Visual C++ Redistributable for Visual Studio 2015.
Either use vc_redist.x64.exe in executable zip or [download it](https://www.microsoft.com/en-us/download/details.aspx?id=48145) from Microsoft.

# Supplementary Materials

Supplementary figures can be found in following directories.

- [DistributionFittingComparisons](https://github.com/ati-ozgur/RmSAT-CFAR/tree/master/SupplementaryMaterials/DistributionFittingComparisons)
- [RayleighMixtureversusOthers](https://github.com/ati-ozgur/RmSAT-CFAR/tree/master/SupplementaryMaterials/RayleighMixtureversusOthers)
- [RayleighMixtureversusRayleigh](https://github.com/ati-ozgur/RmSAT-CFAR/tree/master/SupplementaryMaterials/RayleighMixtureversusRayleigh)


# Data Details
Data Details used in the article can be found in the following [file](https://raw.githubusercontent.com/ati-ozgur/RmSAT-CFAR/master/SupplementaryMaterials/data/DataDetails.txt)






This folder contains the project files to compile FreeLing under MS Visual C++.

These files have been build and provided by Stanilovsky Evgeny.
If you have questions or suggerences regarding freeling MSVC project files, 
please address him to arzamas123 [at] mail.ru.

If you don't need to build FreeLing in windows and only want to use it as an 
out-of-the-box analyzer, you should download and install a binary package 
(instead of the source package you got if you are reading this) and follow 
the README there.

The rest of this file is about how to compile FreeLing in Windows with MSVC 10.


# A. PRE-REQUISITES:

  You will need libicu and libboost (and zlib for FreeLing >= 3.1),
  which do not provide suitable windows binaries.

  You have two options:

  1) Go to FreeLing dowload section and get the
     binary package freeling_win_libraries.zip

  2) Compile them yourself, following the instructions below


 ## OPTION 1: Download freeling_win_libraries.zip

   - Go to download section in FreeLing webpage.
   - Download the binary package freeling_win_libraries.zip
   - Uncompress the file in the directory where you want the 
     libraries installed.    
 

 ## OPTION 2: Compile libicu and libboost (and zlib for FreeLing > 3.1)

  2a) Download ICU, http://site.icu-project.org/ -> ICU4C Download (now testing on icu49)

     * If you use MSVC2010, download binaries or if you want Debug
       symbols - download sources, solution files for MSVC2010 you can find at :
       source/allinone/allinone.sln

  2b) Download Boost, http://boost.org

     * read : http://www.boost.org/doc/libs/1_48_0/libs/regex/doc/html/boost_regex/install.html

      Important:

      ICU is a C++ library just like Boost is, as such your copy of
      ICU must have been built with the same C++ compiler (and
      compiler version) that you are using to build Boost.
      Boost.Regex will not work correctly unless you ensure that this
      is the case: it is up to you to ensure that the version of ICU
      you are using is binary compatible with the toolset you use to
      build Boost.

     * run bootstrap.bat
     * bjam -sICU_PATH=C:\Projects\work\freeling\windows_bin\icu --toolset=msvc-10.0 install threading=multi link=static variant=release --without-python --without-chrono --without-serialization --without-wave --without-signals --without-system --without-math
     * all libs are in bin.v2 directory, just copy them into separate directory

  2c) Download zlib http://www.zlib.net
      cd zlib-1.2.8\contrib\vstudio\vc10\
      and build it.


# B. INSTALLING FREELING FROM BINARY PACKAGES

   - Install pre-requisite libraries as described above (only for FreeLing >= 3.1)
   - Go to download section in FreeLing webpage.
   - Download the binary package freeling_win.zip
   - Uncompress the file in the directory where you want 
     freeling installed.

  You will have a working FreeLing version which you can call from
  your programs or from the test programs included in the source
  tarball (check project files in msvc/10.0/tests)

  For details about how to call the "analyzer.exe" sample program, check FreeLing 
  user manual and/or the README in the windows binary package.

  If you want to compile FreeLing anyway (e.g. because you need a newer version than
  that provided in the binary), see COMPILING FREELING below.


# C. COMPILING FREELING:

  Before compiling FreeLing, make sure you installed the required libraries, following
  one of the options in section PRE-REQUISITES above.  

  Before compiling FreeLing, you need to add BOM (byte-order-markers) to all source files
  so that MSVC is able to read some multibyte characters in string literals. 
  
  To do this, you need to compile and run the simple bom-add.cpp program you will find in
  this package, in the project bom-add-sln. 
  This program recursively adds BOM to all source files in the directory given as a parameter.
  So you need to run e.g. :   bom-add.exe c:\freeling\src

  After this, you are ready to compile FreeLing. Open MSVC 2010 and follow the steps:

    * Load project freeling.sln, 
 
    * Go to Project->Properties->C++->General and set the path to your
      icu, boost, and zlib "include" directory (either extracted from
      freeling_win_libraries or compiled by yourself).

    * Go to Project->Properties->Linker->General->Additional Library
      Directories and set the path to your icu, boost, and zlib "lib"
      directory (either extracted from freeling_win_libraries or
      compiled by yourself).

    * Build freeling.


== Stanilovsky Evgeny  <arzamas123(@)[mail.ru]>.

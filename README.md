# Memory-Efficient Upstream Flow Length (MEUFL)

Part of the [Memory-Efficient I/O-Improved Drainage Analysis System (MIDAS)](https://github.com/HuidaeCho/midas)

**Citation**: Huidae Cho, September 2026. Flow in Float: Memory-Efficient Upstream Flow Length Parallel Computation Using an IEEE-754-Based Union Encoding. Environmental Modelling & Software 204, 107045. [doi:10.1016/j.envsoft.2026.107045](https://doi.org/10.1016/j.envsoft.2026.107045). [Author's Version](https://idea.isnew.info/publications/Flow%20in%20float%20-%20Memory-efficient%20upstream%20flow%20length%20parallel%20computation%20using%20an%20IEEE-754-based%20union%20encoding%20-%20Cho.2026.pdf).

Predefined flow direction encodings in GeoTIFF: power2 (default, r.terraflow, ArcGIS), taudem (d8flowdir), 45degree (r.watershed), degree<br>
![image](https://github.com/user-attachments/assets/990f0530-fded-4ee5-bfbb-85056a50ca1c)
![image](https://github.com/user-attachments/assets/a02dfc15-a825-4210-82c4-4c9296dafadc)
![image](https://github.com/user-attachments/assets/64f5c65a-c7cc-4e06-a69f-6fccd6435426)
![image](https://github.com/user-attachments/assets/fafef436-a5f2-464a-89a8-9f50a877932c)

Custom flow direction encoding is also possible by passing `-e E,SE,S,SW,W,NW,N,NE` (e.g., 1,8,7,6,5,4,3,2 for taudem).

## Test data

You can find test data at https://data.isnew.info/meufl.html.

## How to build on Linux

1. First, install the [GDAL](https://gdal.org/) library.
2. Build meufl
```bash
make
```
or
```bash
mkdir build
cd build
cmake ..
make
```

## How to build on Windows

1. Install [Visual Studio Community Edition](https://visualstudio.microsoft.com/vs/community/). Select these two components:
   * MSVC v143 - VS 2022 C++ x64/x86 build tools (Latest)
   * Windows 11 SDK (10.0.26100.0)
2. Install [Git for Windows](https://gitforwindows.org/)
3. Install [Miniconda](https://www.anaconda.com/download/success)
```cmd
curl -O https://repo.anaconda.com/miniconda/Miniconda3-latest-Windows-x86_64.exe
Miniconda3-latest-Windows-x86_64.exe /S /D=C:\opt\miniconda
C:\opt\miniconda\condabin\conda.bat init
```
4. Start Developer Command Prompt for VS 2022
5. Setup Conda for MEUFL build
```cmd
conda config --add channels conda-forge
conda config --set channel_priority strict
conda create -n meufl cmake libgdal
conda activate meufl
```
6. Download the source code
```cmd
cd \opt
git clone git@github.com:HuidaeCho/meufl.git
cd meufl
mkdir build
cd build
```
7. Build meufl
```cmd
cmake ..
msbuild meufl.sln -p:configuration=release
```
or
```cmd
cmake ..
cmake --build . --config Release
```
or
```cmd
cmake -DCMAKE_CONFIGURATION_TYPES=Release
msbuild meufl.sln
```
8. Find meufl.exe in `Release\meufl.exe`

Copy-Item $PSScriptRoot\..\..\lib\zip.h $PSScriptRoot\NuGetPack\build\native\include\zip.h
Copy-Item $PSScriptRoot\..\..\README.md $PSScriptRoot\NuGetPack\README.md
Copy-Item $PSScriptRoot\..\..\LICENSE $PSScriptRoot\NuGetPack\LICENSE.txt
nuget pack NuGetPack
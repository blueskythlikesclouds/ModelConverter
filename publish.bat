7z x Dependencies/assimp.7z -oDependencies -y
mkdir Publish
msbuild Source/ModelConverter.sln -p:Configuration=Release -p:Platform=x86
msbuild Source/ModelConverter.sln -p:Configuration=Release -p:Platform=x64
7z a Publish/ModelConverter-x86.7z ./Source/ModelConverter/bin/Win32/Release/ModelConverter.exe ./Resources/*
7z a Publish/ModelConverter-x64.7z ./Source/ModelConverter/bin/x64/Release/ModelConverter.exe ./Resources/*
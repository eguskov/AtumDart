@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

cd gen

set ProjectDir=%1%
set AtumDir=%ProjectDir%..\..\Atum\Atum
set DartOutDir=%ProjectDir%..\build\lib

set Targets=Services\Scene\SceneObject.h
set Targets=%Targets%;Services\Scene\Scene.h
set Targets=%Targets%;SceneObjects\PhysBox.h
set Targets=%Targets%;SceneObjects\Terrain.h
rem set Targets=%Targets%;SceneObjects\HoverTank.h

for %%i in (%Targets%) do call :doCodegen %%i
goto :eof

:doCodegen
  set Source=%1
  set Target=%~n1
  echo [Codegen]: %Target%
  Codegen.exe %Target%.rules.json %AtumDir%\%Source% %Target% %DartOutDir%\%Target%.gen.dart
goto :eof
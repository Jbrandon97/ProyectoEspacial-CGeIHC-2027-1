@echo off
setlocal
set "APP=%~dp0bin\x64\Release\ProyectoEspacial.exe"
if not exist "%APP%" (
  echo No existe el ejecutable. Compila ProyectoEspacial.sln en Release x64 desde Visual Studio 2022.
  pause
  exit /b 1
)
pushd "%~dp0bin\x64\Release"
"%APP%"
set "RESULTADO=%ERRORLEVEL%"
popd
exit /b %RESULTADO%

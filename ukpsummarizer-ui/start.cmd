:: Created by npm, please don't edit manually.
@ECHO OFF

SETLOCAL

SET "NPM_EXE=%~dp0\node\npm.cmd"
IF NOT EXIST "%NPM_EXE%" (
  SET "NPM_EXE=npm"
)

SET argC=0
FOR %%x IN (%*) DO SET /A argC+=1

SET args=%*
IF %argC% LSS 1 (
  SET args="start:dev"
)

"%NPM_EXE%" run %args%

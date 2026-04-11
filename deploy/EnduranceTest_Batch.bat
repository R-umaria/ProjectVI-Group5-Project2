@echo off
SET /A "index = 1"
SET /A "count = 100"

:while
@echo %time%
     :spawnloop
     if %index% leq %count% (
          START /MIN Client.exe
          SET /A index = %index% + 1
          @echo %index%
          goto :spawnloop
          )
     timeout 250 > NUL
     SET /A index = 1
     goto :while

@echo off
echo ========================================
echo TwiST Framework - Clean GitHub Push
echo ========================================
echo.
echo SVARBU: Pries paleidziant si skripta:
echo 1. Istrink sena GitHub repository: https://github.com/Voldemaras/TwiST
echo    Settings ^> Danger Zone ^> Delete this repository
echo.
echo 2. Sukurk NAUJA tuscia repository:
echo    - Repository name: TwiST
echo    - Description: Production-ready ESP32 robotics framework
echo    - Public
echo    - NO README, NO .gitignore, NO LICENSE (viskas tuscia!)
echo.
pause

cd /d "%~dp0"

echo.
echo Removing old remote...
git remote remove origin

echo.
echo Adding new clean repository...
git remote add origin https://github.com/Voldemaras/TwiST.git

echo.
echo Pushing clean history...
git push -u origin main

echo.
echo Pushing tag v1.2.0...
git push origin v1.2.0

echo.
echo ========================================
echo SUCCESS! Clean repository uploaded!
echo ========================================
echo.
echo Check: https://github.com/Voldemaras/TwiST/graphs/contributors
echo Should show ONLY: Voldemaras (1 contributor)
echo.
pause

Компиляция библиотек:
gcc -o lib7One.so -shared -fPIC lib7One.c
gcc -o lib7Two.so -shared -fPIC lib7Two.c
Компиляция progOne (статическая компоновка):
gcc -I. progOne.c -L. -l7One -Wl,-rpath,'$ORIGIN' -o progOne
Компиляция progOne (динамическая компоновка):
gcc -o progTwo -ldl progTwo.c

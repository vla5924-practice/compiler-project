# Сборка и установка LLVM


## Полезные ссылки

* [Страница релиза LLVM 13.0.0](https://github.com/llvm/llvm-project/releases/tag/llvmorg-13.0.0)
* [Начало работы с LLVM](https://llvm.org/docs/GettingStarted.html#getting-started-with-llvm)


## Необходимые компоненты

* CMake версии 3.13.4 или новее
* MSVC, gcc или другой совместимый компилятор
* Python 3

Ознакомьтесь с информацией о требованиях на [странице проекта](https://github.com/llvm/llvm-project/blob/release/13.x/README.md).

На Ubuntu необходимые зависимости могут быть установлены с помощью apt:

```sh
sudo apt update
sudo apt install cmake git build-essential libncurses5-dev python3 zlib1g-dev
```


## Установка из прекомпилированных файлов

### Windows

На Windows указанным способом можно только установить компилятор clang. Библиотеку LLVM необходимо собирать вручную.

Скачайте и запустите [установщик](https://github.com/llvm/llvm-project/releases/download/llvmorg-13.0.0/LLVM-13.0.0-win64.exe).


### Ubuntu

На Ubuntu можно избежать ручной сборки. Вы можете скачать подходящий архив со страницы релиза. Для Ubuntu 20.04 архив можно скачать [здесь](https://github.com/llvm/llvm-project/releases/download/llvmorg-13.0.0/clang+llvm-13.0.0-x86_64-linux-gnu-ubuntu-20.04.tar.xz).

```sh
wget https://github.com/llvm/llvm-project/releases/download/llvmorg-13.0.0/clang+llvm-13.0.0-x86_64-linux-gnu-ubuntu-20.04.tar.xz
tar xf clang+llvm-13.0.0-x86_64-linux-gnu-ubuntu-20.04.tar.xz
mv clang+llvm* clang+llvm
```

После этого необходимо изменить переменные окружения:

```sh
export LLVM_DIR=$PWD/clang+llvm/lib/cmake/llvm
export LD_LIBRARY_PATH=$PWD/clang+llvm/lib:$LD_LIBRARY_PATH
export PATH=$PWD/clang+llvm/bin:$PATH
```


## Сборка

1. [Скачайте](https://github.com/llvm/llvm-project/archive/refs/tags/llvmorg-13.0.0.zip) и распакуйте архив с исходным кодом LLVM 13.0.0.
2. Создайте папки для сборки и размещения установленных файлов (например, внутри папки с кодом `/path/to/llvm-project`):
   ```sh
   cd /path/to/llvm-project
   mkdir build
   mkdir install
   ```
3. Запустите CMake:
   ```sh
   cmake -DCMAKE_INSTALL_PREFIX=/path/to/llvm-project/install -DLLVM_PARALLEL_LINK_JOBS=1 -DLLVM_TARGETS_TO_BUILD=X86 -DCMAKE_CXX_STANDARD=17 -Thost=x64 ../llvm
   ```
4. Запустите сборку:
   ```sh
   cmake --build .
   ```
5. После окончания сборки запустите установку:
   ```sh
   cmake --build . --target install
   ```

После установки необходимо изменить переменные окружения: `LLVM_DIR` должно быть `/path/to/llvm-project/install/lib/cmake/llvm`.

[_Назад_](README.md)

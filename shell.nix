{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = [
    # Native build tools
    pkgs.gcc
    pkgs.gnumake
    pkgs.raylib
    pkgs.raygui
    pkgs.xorg.libX11
    pkgs.mesa.drivers
    pkgs.zlib

    # Web build tools
    pkgs.emscripten
    pkgs.git
  ];

  shellHook = ''
    export CC=gcc
    export EMCC=${pkgs.emscripten}/bin/emcc
    export EMAR=${pkgs.emscripten}/bin/emar
    
    # Native build paths
    export CFLAGS="-Wall -I${pkgs.raylib}/include -I${pkgs.raygui}/include"
    export LDFLAGS="-L${pkgs.raylib}/lib"
    export LIBS="-lraylib -lGL -lm -lpthread -ldl -lrt -lX11"

    mkdir -p src build build_web

    echo "Environment ready for building."
    echo "Run 'make' for native build"
    echo "Run 'make web' for web build"
  '';
}
{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = [
    pkgs.gcc
    pkgs.raylib
    pkgs.raygui
    pkgs.xorg.libX11
    pkgs.mesa.drivers
    pkgs.zlib
  ];

  shellHook = ''
    export CC=gcc
    export CFLAGS="-Wall -I${pkgs.raylib}/include -I${pkgs.raygui}/include"
    export LIBS="-lraylib -lGL -lm -lpthread -ldl -lrt -lX11"

    mkdir -p src bin

    echo "Environment ready for building your C project."
    echo "Run 'make' to build the project."
  '';
}
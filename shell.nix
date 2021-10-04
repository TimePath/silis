{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  buildInputs = with pkgs; [
    clang_12
    gcc10

    ccache
    cppcheck
    cpplint
    include-what-you-use

    openjdk16
  ];
}

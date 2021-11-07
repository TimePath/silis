{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  buildInputs = with pkgs; [
    clang_12 llvmPackages_12.bintools
    gcc10

    linuxPackages.perf

    ccache
    cppcheck
    cpplint
    include-what-you-use

    openjdk16
  ];
}

{ pkgs, stdenv, lib, nix-gitignore
, debug ? false
}:
stdenv.mkDerivation {
    name = "silis";
    src = nix-gitignore.gitignoreFilterRecursiveSource (_: _: true) [".git" ".gitignore" "*.nix"] ./.;
    nativeBuildInputs = [ pkgs.cmake_2_8 ];
    installPhase = ''
        mkdir -p $out/bin
        mv silisc $out/bin
    '';
    dontStrip = if debug then true else false;
    cmakeBuildType = if debug then "Debug" else "Release";
    allowSubstitutes = false; # prefer local builds
    meta = {
        description = "static interchange lisp in stages";
        homepage = "https://github.com/TimePath/silis";
        license = stdenv.lib.licenses.mit;
        platforms = stdenv.lib.platforms.linux;
        maintainers = [ stdenv.lib.maintainers.timepath ];
    };
}

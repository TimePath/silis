{ pkgs, buildPackages, stdenv, lib, nix-gitignore
, useCmake ? true
, debug ? false
}:
let
    build = if useCmake then buildCMake else buildMake;
    buildCMake = {
        nativeBuildInputs = [ buildPackages.cmake ];
    };
    buildMake = {
        patchPhase = ''
            ./amalgamate.sh
            echo -e "silisc: silis.o\n\t$CC -o silisc silis.o" > Makefile
        '';
    };
in
stdenv.mkDerivation (build // {
    name = "silis";
    src = nix-gitignore.gitignoreFilterRecursiveSource (_: _: true) [".git" ".gitignore" "*.nix"] ./.;
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
})

{pkgs, stdenv, lib, debug ? false}:
let
    cleanSourceFilter = name: type: let
        baseName = baseNameOf (toString name);
        result = (lib.cleanSourceFilter name type)
            && !(lib.hasSuffix ".nix" baseName)
            && !(type == "directory" && baseName == ".idea")
            && !(type == "directory" && baseName == "cmake-build-debug")
            && !(type == "directory" && baseName == "cmake-build-release")
        ;
    in result;
    src = builtins.filterSource cleanSourceFilter ./.;
in
stdenv.mkDerivation {
    name = "silis";
    src = src;
    nativeBuildInputs = [ pkgs.cmake_2_8 ];
    installPhase = ''
        mkdir -p $out/bin
        mv silis $out/bin
    '';
    dontStrip = if debug then true else false;
    cmakeBuildType = if debug then "Debug" else "Release";
    meta = {
        description = "static interchange lisp in stages";
        homepage = "https://github.com/TimePath/silis";
        license = stdenv.lib.licenses.mit;
        platforms = stdenv.lib.platforms.linux;
        maintainers = [ stdenv.lib.maintainers.timepath ];
    };
}

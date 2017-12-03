{pkgs, pkgsi686Linux, stdenv, lib}:
let
    cleanSourceFilter = name: type: let
        baseName = baseNameOf (toString name);
        result = (lib.cleanSourceFilter name type)
            && !(lib.hasSuffix ".nix" baseName)
            && !(type == "directory" && baseName == ".idea")
        ;
    in result;
    src = builtins.filterSource cleanSourceFilter ./.;
in
stdenv.mkDerivation {
    name = "silis";
    src = src;
    nativeBuildInputs = [ pkgs.cmake ];
    installPhase = ''
        mkdir -p $out/bin
        mv silis $out/bin
    '';
    meta = {
        description = "static interchange lisp in stages";
        homepage = "https://github.com/TimePath/silis";
        license = stdenv.lib.licenses.mit;
        platforms = stdenv.lib.platforms.linux;
        maintainers = [ stdenv.lib.maintainers.timepath ];
    };
}

# nix-build -A silis release.nix
# nix-build -A silis-musl-static release.nix
let
    config = {
        packageOverrides = pkgs: {
            silis = pkgs.callPackage ./default.nix {};
        };
    };
    pkgs = import <nixpkgs> { inherit config; };
in
let
inherit (pkgs) lib stdenv;
identity = it: it;
m = buildMatrix { name = [ "cc" "libc" "link" "type" ]; apply = [ "link" "libc" "cc" "type" ]; } (with pkgs; {
    cc = {
        default = identity;

        gcc = pkg: pkg.override { stdenv = overrideCC stdenv gcc; };
        gcc5 = pkg: pkg.override { stdenv = overrideCC stdenv gcc5; };
        gcc6 = pkg: pkg.override { stdenv = overrideCC stdenv gcc6; };
        gcc7 = pkg: pkg.override { stdenv = overrideCC stdenv gcc7; };

        clang = pkg: pkg.override { stdenv = overrideCC stdenv clang; };
        clang4 = pkg: pkg.override { stdenv = overrideCC stdenv clang_4; };
        clang5 = pkg: pkg.override { stdenv = overrideCC stdenv clang_5; };
        clang6 = pkg: pkg.override { stdenv = overrideCC stdenv clang_6; };
    };
    libc = {
        default = pkg: pkg.overrideAttrs (oldAttrs: {
            buildInputs = (oldAttrs.buildInputs or [])
                ++ lib.optionals (pkg.stdenv.isStatic or false) [ pkgs.glibc.static ];
        });
        musl = pkg: pkg.overrideAttrs (oldAttrs: {
            cmakeFlags = [
                " -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON"
            ];
            CFLAGS = [
                "-isystem ${pkgs.musl}/include"
                "-L${pkgs.musl}/lib"
                "-B${pkgs.musl}/lib"
            ];
        });
    };
    link = {
        default = identity;
        static = pkg: let
            stdenv = makeStaticBinaries pkg.stdenv;
        in (pkg.override { inherit stdenv; }) // { inherit stdenv; };
    };
    type = {
        default = identity;
        debug = pkg: pkg.override { debug = true; };
    };
});
buildMatrix = { name ? apply, apply ? name }: m: prefix: pkg: let
    next = decided: alt: let
        altAttrs = lib.attrNames alt;
    in if (lib.length altAttrs) != 0
        then let
            k = lib.head altAttrs;
        in lib.flatten (map (it: next (decided // { ${k} = it; }) (removeAttrs alt [k])) alt.${k})
        else let
            flavor = it: if it == "default" then "" else "-${it}";
            flags = lib.concatStrings (map (it: flavor decided.${it}) name);
            pkg' = lib.foldl (pkg: f: m.${f}.${decided.${f}} pkg) pkg apply;
        in { name = prefix + flags; value = pkg'.overrideAttrs (oldAttrs: { name = oldAttrs.name + flags; }); }
    ;
    x = lib.listToAttrs (builtins.map (it: { name = it; value = lib.attrNames m.${it}; }) apply);
in lib.listToAttrs (next {} x);
in (m "silis" pkgs.silis) // {

}

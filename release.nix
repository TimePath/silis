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
with pkgs; let
identity = it: it;
m = {
    cc = {
        default = identity;

        gcc = pkg: pkg.override { stdenv = overrideCC stdenv gcc; };
        gcc6 = pkg: pkg.override { stdenv = overrideCC stdenv gcc6; };
        gcc7 = pkg: pkg.override { stdenv = overrideCC stdenv gcc7; };

        clang = pkg: pkg.override { stdenv = overrideCC stdenv clang; };
        clang4 = pkg: pkg.override { stdenv = overrideCC stdenv clang_4; };
        clang5 = pkg: pkg.override { stdenv = overrideCC stdenv clang_5; };
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
};
build' = pkg: cc: libc: link: (cc (libc (link pkg)));
build = cc: libc: link: (build' silis cc libc link);
flavor = it: if it == "default" then "" else "-${it}";
all = lib.listToAttrs (lib.collect (it: it?name) (
    lib.mapAttrs (cc: _:
    lib.mapAttrs (libc: _:
    lib.mapAttrs (link: _:
        let
            flags = (flavor cc) + (flavor libc) + (flavor link);
            pkg = (build m.cc.${cc} m.libc.${libc} m.link.${link});
        in lib.nameValuePair ("silis" + flags) (pkg.overrideAttrs (oldAttrs: { name = oldAttrs.name + flags; }))
    ) m.link
    ) m.libc
    ) m.cc
));
in all // {

}

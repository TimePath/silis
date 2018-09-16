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

        tcc = pkg: (pkg.override { stdenv = overrideCC stdenv tinycc; }).overrideAttrs (oldAttrs: {
            CC = "tcc";
            patchPhase = ''
                grep -l -R "#pragma once" src | while read f; do
                   ${casguard} $f | ${pkgs.moreutils}/bin/sponge $f
                done
            '';
        });
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
casguard = pkgs.writeScript "casguard.py" ''
    #!${pkgs.python3}/bin/python
    import sys
    import shutil
    from hashlib import sha256

    def hash(path):
      h = sha256()
      with open(path, "rb", buffering = 0) as f:
        for b in iter(lambda: f.read(128 * 1024), b""):
          h.update(b)
      return h.hexdigest()

    def cat(path):
      with open(path, "r") as f:
        shutil.copyfileobj(f, sys.stdout)

    path = sys.argv[1]
    guard = "INCLUDED_" + hash(path)[0:32]

    print(f"#ifndef {guard}")
    print(f"#define {guard}")
    print("")
    cat(path)
    print("")
    print("#endif")
'';
in (m "silis" pkgs.silis) // {
    casguard = casguard;
}

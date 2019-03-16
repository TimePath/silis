# nix-build release.nix -A silis
# nix-build release.nix -A silis-musl-static
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
        # gcc48 = pkg: pkg.override { stdenv = overrideCC stdenv gcc48; };
        # gcc49 = pkg: pkg.override { stdenv = overrideCC stdenv gcc49; };
        gcc5 = pkg: pkg.override { stdenv = overrideCC stdenv gcc5; };
        gcc6 = pkg: pkg.override { stdenv = overrideCC stdenv gcc6; };
        gcc7 = pkg: pkg.override { stdenv = overrideCC stdenv gcc7; };
        gcc8 = pkg: pkg.override { stdenv = overrideCC stdenv gcc8; };

        clang = pkg: pkg.override { stdenv = overrideCC stdenv clang; };
        clang37 = pkg: pkg.override { stdenv = overrideCC stdenv clang_37; };
        clang38 = pkg: pkg.override { stdenv = overrideCC stdenv clang_38; };
        clang39 = pkg: pkg.override { stdenv = overrideCC stdenv clang_39; };
        clang4 = pkg: pkg.override { stdenv = overrideCC stdenv clang_4; };
        clang5 = pkg: pkg.override { stdenv = overrideCC stdenv clang_5; };
        clang6 = pkg: pkg.override { stdenv = overrideCC stdenv clang_6; };
        clang7 = pkg: pkg.override { stdenv = overrideCC stdenv clang_7; };

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
            makeStaticBinaries = stdenv: stdenv // {
                mkDerivation = args:
                if stdenv.hostPlatform.isDarwin
                then throw "Cannot build fully static binaries on Darwin/macOS"
                else stdenv.mkDerivation (args // {
                    NIX_CFLAGS_LINK = toString (args.NIX_CFLAGS_LINK or "") + "-static";
                    configureFlags = (args.configureFlags or []) ++ [
                        "--disable-shared" # brrr...
                    ];
                });
            };
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
        result = if (lib.length altAttrs) != 0 then edge else terminal;
        edge = let
            k = lib.head altAttrs;
            result = lib.flatten (map (it: next (decided // { ${k} = it; }) (removeAttrs alt [k])) alt.${k});
            in result;
        terminal = let
            flavor = it: if it == "default" then "" else "-${it}";
            flags = lib.concatStrings (map (it: flavor decided.${it}) name);
            pkg' = lib.foldl (pkg: f: m.${f}.${decided.${f}} pkg) pkg apply;
            result = {
                name = prefix + flags;
                value = pkg'.overrideAttrs (oldAttrs: { name = oldAttrs.name + flags; });
            };
            in result;
        in result;
    alt = lib.listToAttrs (builtins.map (it: { name = it; value = lib.attrNames m.${it}; }) apply);
    result = lib.listToAttrs (next {} alt);
    in result;
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
targets = removeAttrs (m "silis" pkgs.silis) [
    "silis-tcc-musl"
    "silis-tcc-musl-debug"
    "silis-tcc-musl-static"
    "silis-tcc-musl-static-debug"

    "silis-static"
    "silis-static-debug"
    "silis-musl-static"
    "silis-musl-static-debug"
];
result = targets // {
    casguard = casguard;
};
in result

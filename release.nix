# nix-env -f release.nix -qa
# nix build -f release.nix --no-link silis
# nix-build release.nix --no-out-link -A silis
{ crossSystem ? null /* --argstr crossSystem (mingw32 | mingwW64) */ }:
let
    # nixpkgs = <nixpkgs>;
    nixpkgs = builtins.fetchTarball {
        name = "nixos-19.03";
        # `git ls-remote https://github.com/nixos/nixpkgs-channels nixos-19.03`
        url = "https://github.com/nixos/nixpkgs/archive/705986f5a986be5c5ae13193b487c7ec8ca05f16.tar.gz";
        # `nix-prefetch-url --unpack <url>`
        sha256 = "0zpch2cpl2yx0mp7hnyjd03hqs7rxza9wc2p97njsdzhi56gxwxp";
    };
    systems = (import nixpkgs {}).lib.systems.examples;
    pkgs = import nixpkgs {
        config = {
            allowUnsupportedSystem = true;
            packageOverrides = pkgs: {
                silis = pkgs.callPackage ./default.nix { useCmake = crossSystem == null; };
            };
        };
        crossSystem = if crossSystem == null then null else systems.${crossSystem};
    };
in
let
inherit (pkgs) lib stdenv;
identity = it: it;
m = buildMatrix { name = [ "cc" "libc" "link" "type" ]; apply = [ "libc" "cc" "type" "link" ]; } (with pkgs; {
    cc = {
        default = identity;

        emscripten = pkg: (pkg.override { stdenv = emscriptenStdenv; }).overrideAttrs (old: {
            nativeBuildInputs = [ pkgs.cmake ] ++ old.nativeBuildInputs;
            installPhase = ''
                mkdir -p $out/bin
                mv silisc.js $out/bin
            '';
            NIX_CFLAGS_COMPILE = "";
            configurePhase = ''
                HOME=$TMPDIR
                # cmakeConfigurePhase

                runHook preConfigure

                mkdir -p build
                cd build
                cmakeDir=''${cmakeDir:-..}

                cmakeFlags="-DCMAKE_BUILD_TYPE=$cmakeBuildType $cmakeFlags"

                echo "cmake flags: $cmakeFlags ''${cmakeFlagsArray[@]}"
                emconfigure cmake $cmakeDir

                runHook postConfigure
            '';
            checkPhase = "";
        });

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
            cmakeFlags = (oldAttrs.cmakeFlags or []) ++ [
                " -DTinyCC:BOOL=ON"
            ];
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
            CFLAGS = [
                "-isystem ${pkgs.musl}/include"
                "-L${pkgs.musl}/lib"
                "-B${pkgs.musl}/lib"
            ];
        });
    };
    link = {
        default = identity;
        static = pkg: pkgs.pkgsStatic.stdenv.mkDerivation pkg.drvAttrs;
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
    # tcc-musl-*-*
    "silis-tcc-musl"
    "silis-tcc-musl-debug"
    "silis-tcc-musl-static"
    "silis-tcc-musl-static-debug"

    # tcc-default-static-*
    "silis-tcc-static"
    "silis-tcc-static-debug"

    # default-musl-static-*
    "silis-musl-static"
    "silis-musl-static-debug"

    # emscripten-*-*-* except emscripten-default-default-default
    "silis-emscripten-static"
    "silis-emscripten-static-debug"
    "silis-emscripten-musl"
    "silis-emscripten-musl-debug"
    "silis-emscripten-musl-static"
    "silis-emscripten-musl-static-debug"
];
result = targets // {
    casguard = casguard;
};
in result

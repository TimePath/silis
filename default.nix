with import <nixpkgs> {};
(callPackage ./silis.nix {})
.override {
    stdenv = makeStaticBinaries stdenv;
}

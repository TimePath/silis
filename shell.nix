(import ./release.nix {}).silis.overrideAttrs (oldAttrs: {
    hardeningDisable = [ "all" ];
})

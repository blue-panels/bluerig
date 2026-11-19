{
  description = "bluerig, a terminal IDE, a continuation of cooledit";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    { self, nixpkgs }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];
      forAllSystems = f: lib.genAttrs systems (system: f nixpkgs.legacyPackages.${system});

      # A flake has no .git to run "git describe" in, so the version comes
      # from the newest section of CHANGELOG.md and the date of the commit,
      # as nixpkgs names a build of a commit: 1.0.2-unstable-2026-11-19.
      lib = nixpkgs.lib;
      release = lib.removePrefix "## " (
        lib.findFirst (line: builtins.match "## [0-9.]+" line != null) "## 0" (
          lib.splitString "\n" (builtins.readFile ./CHANGELOG.md)
        )
      );
      date = self.lastModifiedDate or "19700101000000";
      version = "${release}-unstable-${builtins.substring 0 4 date}-${builtins.substring 4 2 date}-${builtins.substring 6 2 date}";
    in
    {
      packages = forAllSystems (pkgs: rec {
        # The recipe kept for nixpkgs, with this tree as its source. Its
        # @VERSION@ and @HASH@ only name the release archive, which is not
        # fetched here.
        bluerig = (pkgs.callPackage ./packaging/nix/package.nix.in { }).overrideAttrs {
          inherit version;
          src = self;
          # version.sh reads the version from here outside a Git checkout.
          preConfigure = ''
            echo ${version} > .tarball-version
          '';
          passthru = { };
        };
        default = bluerig;
      });

      devShells = forAllSystems (pkgs: {
        default = pkgs.mkShell {
          inputsFrom = [ self.packages.${pkgs.stdenv.hostPlatform.system}.bluerig ];
        };
      });
    };
}

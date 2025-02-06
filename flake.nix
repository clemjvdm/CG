# Use `nix run .#default` to enter the development environment.
{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem
      (system:
        let
          pkgs = import nixpkgs { inherit system; };
        in
        with pkgs;
        {
          apps.default = {
            type = "app";
            program = let drv = pkgs.buildFHSEnv {
              name = "qt-env";
                targetPkgs = pkgs: (with pkgs; [
                  qt6.full 
                  mesa
                  libGL
                  cmake
                  qtcreator
                  pkgs.cmake-format
                  clang
                  clang-tools
                  ccache
                  gdb
                ]);
              profile = ''
                # Set environment variable that zsh can detect
                export IN_NIX_SHELL=impure
                echo "Entered CG Development Environment"
                echo "Use 'qtcreator &' to open qtcreator"
              '';
            }; in "${drv}/bin/qt-env";
          };
        }
      );
}

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
                  gdb
                ]);
            }; in "${drv}/bin/qt-env";
          };
        }
      );
}

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
          devShells.default = mkShell {
            buildInputs = [ 
              pkgs.qt6.full 
              pkgs.qtcreator
            ];
            shellHook = ''
              echo "Entering Qt6 dev shell"
              echo "Qt Version: ${pkgs.qt6.qtbase.version}"
              echo "Qt Creator Version: ${pkgs.qtcreator.version}"
            '';
          };
        }
      );
}

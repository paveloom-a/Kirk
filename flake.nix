{
  description = "Groovy";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = {
    self,
    nixpkgs,
    flake-utils,
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import nixpkgs {
        inherit system;
      };
      llvm = pkgs.llvmPackages_16;
      ccacheStdenv = pkgs.ccacheStdenv.override {
        stdenv = llvm.stdenv.override {
          cc = llvm.stdenv.cc.override {
            inherit (llvm) bintools;
          };
        };
      };
    in {
      devShells.default = ccacheStdenv.mkDerivation {
        name = "shell";
        nativeBuildInputs = [
          pkgs.bashInteractive
          pkgs.ccache
          pkgs.clang-analyzer
          pkgs.clang-tools_16
          pkgs.cppcheck
          pkgs.cpplint
          pkgs.desktop-file-utils
          pkgs.gnome.devhelp
          pkgs.libxml2
        ];
        buildInputs = [
          pkgs.gtk4
          pkgs.meson
          pkgs.ninja
          pkgs.pkg-config
        ];
        env = {
          ASAN_OPTIONS = "abort_on_error=1:halt_on_error=1";
          LSAN_OPTIONS = "print_suppressions=0:suppressions=../suppr.txt";
          UBSAN_OPTIONS = "abort_on_error=1:halt_on_error=1";
          XDG_DATA_DIRS = pkgs.lib.makeSearchPathOutput "devdoc" "share" [
            pkgs.glib
            pkgs.gtk4
          ];
        };
      };
    });
}

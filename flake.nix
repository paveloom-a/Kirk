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
      llvmStdenv = llvm.stdenv.override {
        cc = llvm.stdenv.cc.override {
          inherit (llvm) bintools;
        };
      };
      ccacheStdenv = pkgs.ccacheStdenv.override {
        stdenv = llvmStdenv;
      };
      nativeBuildInputsRelease = [
        pkgs.blueprint-compiler
        pkgs.desktop-file-utils
        pkgs.libxml2
        pkgs.meson
        pkgs.ninja
        pkgs.pkg-config
      ];
      nativeBuildInputsDebug = [
        pkgs.appstream-glib
        pkgs.bashInteractive
        pkgs.ccache
        pkgs.clang-analyzer
        pkgs.clang-tools_16
        pkgs.cppcheck
        pkgs.cpplint
        pkgs.gnome.devhelp
        pkgs.ltex-ls
        pkgs.shellcheck
        pkgs.yamlfmt
        pkgs.yamllint
      ];
      buildInputs = [
        pkgs.gtk4
      ];
    in {
      devShells.default = ccacheStdenv.mkDerivation {
        name = "groovy-shell";

        inherit buildInputs;

        nativeBuildInputs = nativeBuildInputsRelease ++ nativeBuildInputsDebug;

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
      packages.default = llvmStdenv.mkDerivation {
        pname = "groovy";
        version = "0.1.0";

        src = ./.;

        inherit buildInputs;

        nativeBuildInputs = nativeBuildInputsRelease;

        mesonBuildType = "release";
      };
    });
}

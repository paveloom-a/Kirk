{
  description = "WIP";
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

      nativeBuildInputsRelease = with pkgs; [
        blueprint-compiler
        desktop-file-utils
        libxml2
        meson
        ninja
        pkg-config
        wrapGAppsHook4
      ];

      nativeBuildInputsDebug = with pkgs; [
        alejandra
        appstream-glib
        bashInteractive
        ccache
        clang-analyzer
        clang-tools_16
        cppcheck
        cpplint
        gnome.devhelp
        libadwaita.devdoc
        ltex-ls
        nil
        shellcheck
        valgrind
        yamlfmt
        yamllint
      ];

      buildInputs = with pkgs; [
        libadwaita
      ];
    in {
      devShells.default = ccacheStdenv.mkDerivation {
        name = "kirk-env";

        inherit buildInputs;

        nativeBuildInputs = nativeBuildInputsRelease ++ nativeBuildInputsDebug;

        env = {
          GLIB_SUPP_FILE = "${pkgs.glib.dev}/share/glib-2.0/valgrind/glib.supp";
          GTK_SUPP_FILE = "${pkgs.gtk4}/share/gtk-4.0/valgrind/gtk.supp";
          XDG_DATA_DIRS = pkgs.lib.makeSearchPathOutput "devdoc" "share" [
            pkgs.glib
            pkgs.gtk4
          ];
        };
      };

      packages.default = llvmStdenv.mkDerivation {
        pname = "kirk";
        version = "0.1.0";

        src = ./.;

        inherit buildInputs;

        nativeBuildInputs = nativeBuildInputsRelease;

        mesonBuildType = "release";
      };
    });
}

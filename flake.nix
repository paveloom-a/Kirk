rec {
  description = "WIP";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = {
    self,
    nixpkgs,
    flake-utils,
  }: let
    platforms = [
      "aarch64-linux"
      "x86_64-linux"
    ];
  in
    flake-utils.lib.eachSystem platforms (system: let
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
        stdenv = pkgs.stdenvAdapters.useMoldLinker llvmStdenv;
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
        curl
        gnome.devhelp
        iproute2
        ltex-ls
        netcat
        nettools
        nil
        nix-output-monitor
        nix-tree
        nvd
        shellcheck
        valgrind
        yamlfmt
        yamllint
      ];

      buildInputs = with pkgs; [
        libadwaita
        libsecret
      ];
    in {
      devShells.default = ccacheStdenv.mkDerivation {
        name = "kirk-env";

        nativeBuildInputs = nativeBuildInputsRelease ++ nativeBuildInputsDebug;

        inherit buildInputs;

        env = {
          GLIB_SUPP_FILE = "${pkgs.glib.dev}/share/glib-2.0/valgrind/glib.supp";
          GTK_SUPP_FILE = "${pkgs.gtk4}/share/gtk-4.0/valgrind/gtk.supp";
          XDG_DATA_DIRS = nixpkgs.lib.makeSearchPathOutput "devdoc" "share" (with pkgs; [
            glib
            gtk4
            libadwaita
            libsecret
          ]);
        };
      };

      packages.default = llvmStdenv.mkDerivation {
        pname = "kirk";
        version = "0.1.0";

        src = ./.;

        nativeBuildInputs = nativeBuildInputsRelease;

        inherit buildInputs;

        mesonBuildType = "release";

        meta = with nixpkgs.lib; {
          inherit description;
          homepage = "https://github.com/paveloom-a/Kirk";
          license = licenses.gpl3Plus;
          inherit platforms;
          maintainers = [maintainers.paveloom];
        };
      };
    });
}

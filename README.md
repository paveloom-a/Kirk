### Notices

#### Mirrors

Repository:
- [Codeberg](https://codeberg.org/paveloom-a/Kirk)
- [GitHub](https://github.com/paveloom-a/Kirk)
- [GitLab](https://gitlab.com/paveloom-g/apps/Kirk)

#### Develop

Make sure you have installed the following:

- [Nix](https://nixos.org)
- [`direnv`](https://github.com/direnv/direnv)
- [`nix-direnv`](https://github.com/nix-community/nix-direnv)

Allow `direnv` to load the environment by executing `direnv allow`.

Then, to build in the `debug` mode, run

```bash
meson debug
cd debug
ninja && ./kirk
```

To build in the `release` mode, run

```bash
meson --buildtype=release release
cd release
ninja && ./kirk
```

Alternatively, you can build or run the Nix package via `nix build` and `nix run`, respectively.

You might also want to create a symlink of the `compile_commands.json` file from the `debug` mode build directory in the root of the repository, so that the tooling can see it:

```bash
ln -s debug/compile_commands.json
```

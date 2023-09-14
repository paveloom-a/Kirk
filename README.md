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
meson setup debug
cd debug
ninja && ./kirk
```

To build in the `release` mode, run `nix build` (the result is the target of the `result` symbolic link). To run the build immediately, run `nix run` instead.

You might also want to create a symbolic link of the `compile_commands.json` file from the `debug` mode build directory in the root of the repository, so that the tooling can see it:

```bash
ln -s debug/compile_commands.json
```

To lint the code, run `ninja lint`. To check for memory leaks, run:

```bash
ninja && valgrind \
    --leak-check=full \
    --show-leak-kinds=definite \
    --suppressions=$GLIB_SUPP_FILE \
    --suppressions=$GTK_SUPP_FILE \
    --suppressions=../kirk.supp \
    ./kirk
```

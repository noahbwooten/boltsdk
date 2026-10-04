# An empty application

The least a BoltOS application needs to run: one window, one line of text,
and every export the system calls, each commented with whether it is required
and when it is called. Start here.

```
build.cmd examples\empty             ./build.sh examples/empty
run.cmd examples\empty               ./run.sh examples/empty
```

To start an application of your own, copy this folder anywhere, change the
`[application]` section of `app.ini` — `module` above all, which is the file's
name on the machine — and build the copy. See
[Getting started](../../docs/guide/getting-started.md).

| | |
|---|---|
| `app.ini` | the project |
| `src/main.c` | the application |

Next steps: add controls (`cmnctl2` in `imports`, and see the calculator), an
icon (`[resources]`), and a package (`[package]`).

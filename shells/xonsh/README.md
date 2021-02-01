# xonsh-tcg

This is a [Xonsh](https://xon.sh/) plugin for tcg. To install this plugin, do

```bash
xpip install xonsh-tcg
```

After installation, add

```xonsh
xontrib load tcg
```

to your `.xonshrc` to load it automatically. Then `tcg` will automatically create
cgroup for your shells. The cgoup of the current shell will be displayed in
`$PROMPT` and `$TITLE`.

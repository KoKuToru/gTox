xdg-app
=======

More info about xdg-app [here](https://wiki.gnome.org/Projects/SandboxedApps).

Build
-----
```bash
xdg-app-builder ${BUILD_PATH} org.kokutoru.gtox.json
```

Use local repro
----
```bash
xdg-app build-export ${REPRO_PATH} ${BUILD_PATH}
xdg-app repo-update ${REPRO_PATH}
xdg-app add-remote ${REPRO_NAME} file://${REPRO_PATH} --user --no-gpg-verify
```

Install app
----
```bash
xdg-app install-app ${REPRO_NAME} org.kokutoru.gtox --user
```

Start
-----
```bash
xdg-app run org.kokutoru.gtox
```

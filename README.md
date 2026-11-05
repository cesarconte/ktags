# macOS Finder Color Tags for Dolphin (`ktags`)

**macOS Finder Color Tags (Etiquetas de Colores) integration for Dolphin & KDE Plasma 6.**

Brings the seamless 1-click color tagging and sidebar navigation of macOS Finder to KDE Dolphin with zero indexer overhead and instant responsiveness.

---

## ✨ Features

- **1-Click Dolphin Context Menu**:
  - Right-click any file, folder, or multi-selection in Dolphin to tag:
    - 🔴 **Rojo**
    - 🟠 **Naranja**
    - 🟡 **Amarillo**
    - 🟢 **Verde**
    - 🔵 **Azul**
    - 🟣 **Morado**
    - ⚪ **Gris**
    - 🚫 **Quitar etiquetas**
- **Dolphin Places Sidebar Integration**:
  - Live color categories installed directly into Dolphin's Places sidebar.
  - Clicking any color instantly lists all tagged files in real-time with full file previews.
- **Extended Attributes (`xattr`)**:
  - Standard `user.xdg.tags` and `user.color.tag` metadata written directly to filesystem entries.
- **Ultra-Fast & Zero Background Overhead**:
  - Operates in ~2 milliseconds without needing background indexing daemons.

---

## 🛠️ Compilación e Instalación

```bash
git clone https://github.com/cesarconte/ktags.git
cd ktags
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
sudo cmake --install build
ktag setup-places
```

---

## 📄 Licencia

GPLv3. Desarrollado por César Conte.

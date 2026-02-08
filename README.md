# ptop

![Language](https://img.shields.io/badge/language-C11-blue)
![Platform](https://img.shields.io/badge/platform-Linux-black?logo=linux)
![License](https://img.shields.io/badge/license-GPLv2-blue)
![Size](https://img.shields.io/badge/size-~50KB-brightgreen)
![Dependencies](https://img.shields.io/badge/dependencies-zero-brightgreen)
![Status](https://img.shields.io/badge/status-WIP-orange)

Monitor de recursos de "máxima eficiência" em C puro (sem ncurses), focado em performance extrema e estética minimalista. Inspirado no [btop](https://github.com/aristocratos/btop).

| ptop (WIP) | btop (Original) |
| :---: | :---: |
| ![Tela do ptop](.github/img/ptop.png) | ![Tela do btop](.github/img/btop.webp) |

## Destaques

* **Zero Bloat:** Sem dependências externas (apenas libc).
* **Zero Malloc:** Alocação de memória 100% estática durante a execução.
* **Visual:** Gráficos em Braille, gradientes TrueColor e layout responsivo.
* **Suckless:** Configuração via header file (`cfg.h`) para compilação otimizada por máquina.

## Compilação

O projeto segue a filosofia "suckless". Edite `src/cfg.h` para definir a topologia da sua CPU e compile:

```bash
# 1. Edite a configuração
nano src/cfg.h

# 2. Compile e rode
make release
./ptop
```

## Controles

| Tecla | Ação |
| :---: | :--- |
| `1` | Alternar CPU |
| `2` | Alternar Memória |
| `r` | Forçar Redesenho |
| `q` | Sair |

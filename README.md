# ptop

![Language](https://img.shields.io/badge/language-C11-blue)
![Platform](https://img.shields.io/badge/platform-Linux-black?logo=linux)
![License](https://img.shields.io/badge/license-GPLv2-blue)
![Dependencies](https://img.shields.io/badge/dependencies-zero-brightgreen)
![Status](https://img.shields.io/badge/status-WIP-orange)

Monitor de recursos de alta performance em C puro (sem ncurses), inspirado no [btop](https://github.com/aristocratos/btop).

| ptop (WIP) | btop (Original) |
| :---: | :---: |
| ![Tela do cpumon](.github/img/ptop.png) | ![Tela do btop](.github/img/btop.webp) |

## Destaques
* **Zero Bloat:** Sem dependências externas. Alocação estática e chamadas de sistema otimizadas.
* **Visual:** Gráficos em Braille, gradientes TrueColor e layout responsivo.
* **Monitoramento:** Dados detalhados por núcleo (Temp, Freq, Load) e Uptime.

## Compilação
Edite `src/cfg.h` para definir a topologia da sua CPU (**Threads**, **Cores Físicos**) e compile:

```bash
make
./ptop
```

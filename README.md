# ptop

![Platform](https://img.shields.io/badge/-Linux-black?logo=linux)
![License](https://img.shields.io/badge/-GPLv2-blue)
![Status](https://img.shields.io/badge/-WIP-orange)
![Language](https://img.shields.io/badge/-C11-blue)
![Usage](https://img.shields.io/badge/Usage-System_resource_monitor-yellow)

Monitor de recursos feito em C, focado em performance extrema. Inspirado no [btop](https://github.com/aristocratos/btop).

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
| `q` | Sair |
| `1` | Ligar/Desligar CPU |
| `2` | Ligar/Desligar Memória |
| `r` | Forçar Redesenho |

## License

[GPLv2](LICENSE)

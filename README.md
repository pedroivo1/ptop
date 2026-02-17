# ptop

![Platform](https://img.shields.io/badge/-Linux-black?logo=linux)
![License](https://img.shields.io/badge/-GPLv2-blue)
![Status](https://img.shields.io/badge/-WIP-orange)
![Language](https://img.shields.io/badge/-C11-blue)
![Usage](https://img.shields.io/badge/Usage-System_resource_monitor-yellow)

Personal Tabel of Processes (**ptop**) é um monitor de recursos de sistema focado em performance. Escrito em C, sem dependências externas além da biblioteca padrão.

Inspirado na estética do [btop](https://github.com/aristocratos/btop).

| ptop (WIP) | btop (Original) |
| :---: | :---: |
| ![Tela do ptop](.github/img/ptop.webp) | ![Tela do btop](.github/img/btop.webp) |

## Instalação

```bash
# 1. Clone o repositório
git clone https://github.com/pedroivo1/ptop.git
cd ptop

# 2. Compile (Otimizado para sua arquitetura)
make release

# 3. Execute
./ptop
```

## Controles

| Tecla | Ação |
| :---: | :--- |
| `q` | Sair |
| `1` | Ligar/Desligar CPU |
| `2` | Ligar/Desligar Memória |
| `t` | Trocar Tema |
| `r` | Forçar Redesenho |

## License

[GPLv2](LICENSE)

# ptop

![Platform](https://img.shields.io/badge/-Linux-black?logo=linux)
![License](https://img.shields.io/badge/-GPLv2-blue)
![Status](https://img.shields.io/badge/-WIP-orange)
![Language](https://img.shields.io/badge/-C11-blue)
![Usage](https://img.shields.io/badge/Usage-System_resource_monitor-yellow)

**ptop** é um monitor de recursos de sistema focado em **performance**. Escrito em C puro, sem dependências externas além da biblioteca padrão.

Inspirado na estética do [btop](https://github.com/aristocratos/btop), mas arquitetado para ser leve, modular e eficiente.

| ptop (WIP) | btop (Original) |
| :---: | :---: |
| ![Tela do ptop](.github/img/ptop.webp) | ![Tela do btop](.github/img/btop.webp) |

## Instalação

O projeto segue a filosofia *suckless*. Não há scripts de configuração complexos; o código se adapta ao seu hardware.

```bash
# 1. Clone o repositório
git clone [https://github.com/seu-usuario/ptop](https://github.com/seu-usuario/ptop)
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
| `r` | Forçar Redesenho |

## License

[GPLv2](LICENSE)

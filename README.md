# ptop

![Platform](https://img.shields.io/badge/-Linux-black?logo=linux)
![License](https://img.shields.io/badge/-GPLv2-blue)
![Status](https://img.shields.io/badge/-WIP-orange)
![Language](https://img.shields.io/badge/-C11-blue)
![Usage](https://img.shields.io/badge/Usage-System_resource_monitor-yellow)

Personal Table of Processes (**ptop**) é um monitor de recursos de sistema focado em performance. Escrito em C, sem dependências externas além da biblioteca padrão.

Inspirado na estética do [btop](https://github.com/aristocratos/btop).

| ptop (WIP) | btop (Original) |
| :---: | :---: |
| ![Tela do ptop](.github/img/ptop.webp) | ![Tela do btop](.github/img/btop.webp) |

A parte mais desenvolvida foi o módulo da CPU que já está quase finalizado:

| ptop (cpu only) | btop (cpu only) |
| :---: | :---: |
| ![Tela do ptop](.github/img/ptop-cpu.webp) | ![Tela do btop](.github/img/btop-cpu.webp) |

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

`- 200ms +`: muda o intervalo de atualização.

| Tecla | Ação |
| :---: | :--- |
| `q` | Sair |
| `1` | Ligar/Desligar CPU |
| `2` | Ligar/Desligar Memória |
| `t` | Trocar Tema |
| `r` | Forçar Redesenho |

## Performance

### Eficiência de CPU (Benchmark de 10s)

Em um teste de 10 segundos, o **ptop** utilizou apenas **0.25% de 1 core** de CPU.

| Métrica | Tempo Gasto |
| :--- | ---: |
| **User (Lógica)** | **0.003s** |
| **System (Kernel)** | **0.022s** |
| **Total Ativo** | **0.025s** |
| **Ocioso (Idle)** | 9.975s |

## License

[GPLv2](LICENSE)

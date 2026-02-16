#ifndef PALETTE_LIGHT_H
#define PALETTE_LIGHT_H

// --- UI BASE ---
#define PAL_BG         "255"  // Fundo Branco
#define PAL_FG         "234"  // Texto Preto/Cinza Escuro
#define PAL_DIM        "244"  // Texto Cinza Médio
#define PAL_DIM_DARK   "250"  // Texto Cinza Claro (detalhes)

// --- ANSI COLORS (Tons mais escuros) ---
#define PAL_BLK        "252"  
#define PAL_RED        "124"  // Vermelho Sangue (escuro)
#define PAL_GRN        "28"   // Verde Floresta
#define PAL_YLW        "136"  // Ouro Escuro (Amarelo puro 226 é invisível no branco)
#define PAL_BLU        "18"   // Azul Marinho
#define PAL_MAG        "90"   // Roxo Escuro
#define PAL_CYN        "24"   // Turquesa Escuro
#define PAL_WHT        "232" 

// --- GRADIENT: TEMPERATURE (0-15) ---
// Mudança: De Azul->Ciano->Verde->Amarelo->Vermelho
// Ajuste: Tudo mais escuro para ler no branco
#define PAL_T0  "18"   // Azul Escuro
#define PAL_T1  "24"   // Azul Petróleo
#define PAL_T2  "25"   // Azul
#define PAL_T3  "31"   // Azul Claro (legível)
#define PAL_T4  "30"   // Turquesa Escuro
#define PAL_T5  "36"   // Turquesa
#define PAL_T6  "28"   // Verde Escuro
#define PAL_T7  "34"   // Verde Médio
#define PAL_T8  "64"   // Verde Oliva (substitui o verde limão)
#define PAL_T9  "100"  // Amarelo Queimado/Oliva
#define PAL_T10 "136"  // Ouro (substitui amarelo neon)
#define PAL_T11 "166"  // Laranja
#define PAL_T12 "160"  // Vermelho
#define PAL_T13 "124"  // Vermelho Escuro
#define PAL_T14 "88"   // Vinho
#define PAL_T15 "53"   // Roxo Escuro

// --- GRADIENT: PERCENTAGE (0-7) ---
// Usado nas barras de memória
#define PAL_P0  "28"   // Verde Floresta (era verde claro invisível)
#define PAL_P1  "34"   // Verde Médio
#define PAL_P2  "70"   // Verde Oliva
#define PAL_P3  "136"  // Ouro Escuro (era amarelo invisível)
#define PAL_P4  "166"  // Laranja (era amarelo claro)
#define PAL_P5  "160"  // Vermelho
#define PAL_P6  "124"  // Vermelho Escuro
#define PAL_P7  "90"   // Roxo

// --- SPECIFIC MODULES ---
#define PAL_CPU_BORDER "24"   // Azul Petróleo
#define PAL_MEM_BORDER "22"   // Verde Musgo
#define PAL_MEM_FREE   "242"  // Cinza um pouco mais escuro para a barra vazia

#endif

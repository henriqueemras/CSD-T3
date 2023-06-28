#include <hf-risc.h>

#define TDES_BASE               0xe7000000
#define TDES_CONTROL            (*(volatile uint32_t *)(TDES_BASE + 0x000))
#define TDES_KEY1A              (*(volatile uint32_t *)(TDES_BASE + 0x010))
#define TDES_KEY1B              (*(volatile uint32_t *)(TDES_BASE + 0x020))
#define TDES_KEY2A              (*(volatile uint32_t *)(TDES_BASE + 0x030))
#define TDES_KEY2B              (*(volatile uint32_t *)(TDES_BASE + 0x040))
#define TDES_KEY3A              (*(volatile uint32_t *)(TDES_BASE + 0x050))
#define TDES_KEY3B              (*(volatile uint32_t *)(TDES_BASE + 0x060))
#define TDES_DATA_IN1           (*(volatile uint32_t *)(TDES_BASE + 0x070))
#define TDES_DATA_IN2           (*(volatile uint32_t *)(TDES_BASE + 0x080))
#define TDES_DATA_OUT1          (*(volatile uint32_t *)(TDES_BASE + 0x090))
#define TDES_DATA_OUT2          (*(volatile uint32_t *)(TDES_BASE + 0x0A0))

#define TDES_OUT_RECEIVED       (1 << 7)
#define TDES_MODULE_RESET       (1 << 6)
#define TDES_FUNCTION_SELECT    (1 << 5)
#define TDES_LDKEY              (1 << 4)
#define TDES_LDDATA             (1 << 3)
#define TDES_OUT_READY          (1 << 2)

const uint32_t tdes_key[6] = {
    0xf0e1d2c3, 0xb4a59687, // Chave 1
    0x78695a4b, 0x5a4b7869, // Chave 2
    0x9687b4a5, 0xd2c3f0e1  // Chave 3
};

void tdes_process(uint32_t function_select, uint64_t data_in, uint32_t const key[6]){
    TDES_CONTROL |= TDES_MODULE_RESET;  // Reset do módulo
    TDES_CONTROL &= ~TDES_MODULE_RESET;

    TDES_CONTROL |= TDES_LDKEY;  // Carrega as chaves
    TDES_KEY1A = key[0];
    TDES_KEY1B = key[1];
    TDES_KEY2A = key[2];
    TDES_KEY2B = key[3];
    TDES_KEY3A = key[4];
    TDES_KEY3B = key[5];
    TDES_CONTROL &= ~TDES_LDKEY;

    TDES_DATA_IN1 = data_in >> 32;  // Define o dado de entrada (parte alta)
    TDES_DATA_IN2 = data_in & 0xFFFFFFFF;  // Define o dado de entrada (parte baixa)

    TDES_CONTROL |= TDES_LDDATA;  // Carrega o dado de entrada
    TDES_CONTROL &= ~TDES_LDDATA;

    if (function_select == 0) {
        TDES_CONTROL |= TDES_FUNCTION_SELECT;  // Define a função de encriptação
    } else {
        TDES_CONTROL &= ~TDES_FUNCTION_SELECT;  // Define a função de decriptação
    }

    TDES_CONTROL |= TDES_OUT_RECEIVED;  // Habilita a saída do dado

    while (!(TDES_CONTROL & TDES_OUT_READY));  // Aguarda o sinal de saída pronto

    TDES_CONTROL &= ~TDES_OUT_RECEIVED;  // Desabilita a saída do dado

    // Obtém o resultado da encriptação/decriptação
    uint64_t data_out = ((uint64_t)TDES_DATA_OUT1 << 32) | (uint64_t)TDES_DATA_OUT2;

    printf("%s: %8x%8x\n", (function_select == 0) ? "encipher" : "decipher", (uint32_t)(data_out >> 32), (uint32_t)data_out);
}

int main(void){
    uint64_t msg = 0x1234567890123456;
    uint32_t cycles;

    printf("message: %16llx\n", msg);

    cycles = TIMER0;
    tdes_process(0, msg, tdes_key);  // Encriptação
    cycles = TIMER0 - cycles;
    printf("encipher: %16llx, %d cycles\n", msg, cycles);

    cycles = TIMER0;
    tdes_process(1, msg, tdes_key);  // Decriptação
    cycles = TIMER0 - cycles;
    printf("decipher: %16llx, %d cycles\n", msg, cycles);

    return 0;
}

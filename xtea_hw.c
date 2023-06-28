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

#define TDES_OUT_RECEIVED       (1 << 0)
#define TDES_MODULE_RESET       (1 << 1)
#define TDES_FUNCTION_SELECT    (1 << 2)
#define TDES_LDKEY              (1 << 3)
#define TDES_LDDATA             (1 << 4)
#define TDES_OUT_READY          (1 << 5)

const uint32_t tdes_key[6] = {
    0x00000000, 0x00000000, // Chave 1
    0x11111111, 0x11111111, // Chave 2
    0x22222222, 0x22222222  // Chave 3
};

uint32_t global_data_out1 = 0;
	uint32_t global_data_out2 = 0;

void tdes_process(uint32_t function_select, uint32_t data_in, uint32_t data_in2, uint32_t const key[6]){
    if (function_select == 0) {
        TDES_CONTROL |= TDES_FUNCTION_SELECT;  // Define a função de encriptação
        printf("Decriptando\n");
    } else {
    	printf("Encriptando\n");
        TDES_CONTROL &= ~TDES_FUNCTION_SELECT;  // Define a função de decriptação
    }
//printf("%s: %08x%08x\n", (function_select == 0) ? "encipher" : "decipher", data_out1, data_out2);
    
    //printf("TDES_CONTROL: 0x%08x\n", TDES_CONTROL);
    printf("data_in1: 0x%08x\n", data_in);
    printf("data_in2: 0x%08x\n", data_in2);
    
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

    //printf("TDES_CONTROL: 0x%08x\n", TDES_CONTROL);

    TDES_DATA_IN1 = data_in2;  // Define o dado de entrada (parte alta)
    TDES_DATA_IN2 = data_in;  // Define o dado de entrada (parte baixa)

    TDES_CONTROL |= TDES_LDDATA;  // Carrega o dado de entrada
    TDES_CONTROL &= ~TDES_LDDATA;

    //printf("TDES_CONTROL: 0x%08x\n", TDES_CONTROL);

    while (!(TDES_CONTROL & TDES_OUT_READY));  // Aguarda o sinal de saída pronto

    TDES_CONTROL &= TDES_OUT_RECEIVED;  // Desabilita a saída do dado

    //printf("TDES_CONTROL: 0x%08x\n", TDES_CONTROL);
	 uint32_t data_out1 = TDES_DATA_OUT1;
    uint32_t data_out2 = TDES_DATA_OUT2;

    printf("data_out_new: 0x%08x\n", data_out1);
    printf("data_out2_new: 0x%08x\n", data_out2);
    // Obtém o resultado da encriptação/decriptação
    global_data_out2 = TDES_DATA_OUT1;
    global_data_out1 = TDES_DATA_OUT2;

    //printf("data_out1: 0x%08x\n", global_data_out1);
    //printf("data_out2: 0x%08x\n", global_data_out2);

    
}


int main(void){
    uint32_t msg = 	0x12345678;
    uint32_t msg2 = 	0x12345678;
    // Variável global para armazenar o resultado do data_out

    //uint32_t cycles;

    printf("message: %08x\n", msg);
    printf("message: %08x\n", msg2);

    //cycles = TIMER0;
    tdes_process(0, msg, msg2, tdes_key);  // Encriptação
    //cycles = TIMER0 - cycles;
    printf("in1: %08x\n", global_data_out1);
    printf("in2: %08x\n", global_data_out2);


    //cycles = TIMER0;
    tdes_process(1, global_data_out1, global_data_out2, tdes_key);  // Decriptação
    //cycles = TIMER0 - cycles;
    //printf("out: %08x\n", global_data_out1);
    //printf("out: %08x\n", global_data_out2);

    return 0;
}


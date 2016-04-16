#include <hal.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include <state_machine.h>

struct state_shadow {
  enum state state;
  enum state retstate;
  uint16_t pwm[3];
  uint8_t gpio_set;
  uint8_t gpio_get;

  uint32_t id;
};

extern enum state state;
extern enum state retstate;
extern uint16_t pwm0;
extern uint16_t pwm1;
extern uint16_t pwm2;
extern uint8_t gpio_set;
extern uint8_t gpio_get;

uint8_t CHIPID0 = 0xde;
uint8_t CHIPID1 = 0xca;
uint8_t CHIPID2 = 0xfb;
uint8_t CHIPID3 = 0xad;

void init_state(struct state_shadow *st, uint32_t id) {
  st->state = IDLE;
  st->retstate = IDLE;
  st->pwm[0] = 0;
  st->pwm[1] = 0;
  st->pwm[2] = 0;
  st->gpio_set = 0;
  st->gpio_get = 0;
  
  st->id = id;
}

void save_state(struct state_shadow *st) {
  st->state = state;
  st->retstate = retstate;
  st->pwm[0] = pwm0;
  st->pwm[1] = pwm1;
  st->pwm[2] = pwm2;
  st->gpio_set = gpio_set;
  st->gpio_get = gpio_get;
  assert(st->id == (((uint32_t)CHIPID0 << 24) | ((uint32_t)CHIPID1 << 16) | ((uint32_t)CHIPID2 << 8) | (uint32_t)CHIPID3));
}

void restore_state(const struct state_shadow *st) {
  state = st->state;
  retstate = st->retstate;
  pwm0 = st->pwm[0];
  pwm1 = st->pwm[1];
  pwm2 = st->pwm[2];
  gpio_set = st->gpio_set;
  gpio_get = st->gpio_get;
  
  CHIPID0 = st->id >> 24;
  CHIPID1 = st->id >> 16;
  CHIPID2 = st->id >> 8;
  CHIPID3 = st->id >> 0;
}
  

void hal_reset(void) {
  printf("hal_reset!\n");
  exit(1);
}
static int last_tx = -1;
void hal_uart_tx(uint8_t tx) {
  //printf("tx 0x%02x (%d)\n", tx, tx);
  last_tx = tx;
}
static bool adc_started = false;
void hal_start_adc(void) {
  adc_started = true;
}
static uint16_t next_adc;
uint8_t hal_adc_lsb(void) {
  return next_adc;
}
uint8_t hal_adc_msb(void) {
  return next_adc >> 8;
}

static int last_pwm[3] = {-1, -1, -1};
void hal_set_pwm0(uint16_t set) {
  last_pwm[0] = set;
}
void hal_set_pwm1(uint16_t set) {
  last_pwm[1] = set;
}
void hal_set_pwm2(uint16_t set) {
  last_pwm[2] = set;
}
static uint8_t next_gpio;
uint8_t hal_get_gpio(void) {
  return next_gpio;
}
static uint8_t last_gpio;
void hal_set_gpio(uint8_t set) {
  last_gpio = set;
}

#define EXPECT(exp, got) do { \
    if ((exp) != (got)) { \
      printf("'%s' line %d: Expected %d, got %d\n", #got, __LINE__, exp, got); \
    } } while (0)

#define EXPECT_ECHO(rx) do { \
    last_tx = -1; \
    state_machine_handle_rx_byte(rx); \
    if (last_tx != (rx)) { \
      printf("line %d: Expected echo of 0x%x, got 0x%x.\n", \
	     __LINE__, rx, last_tx); \
    } } while(0)

#define TX_EXPECT(rx, exp) do {			\
    last_tx = -1; \
    state_machine_handle_rx_byte(rx); \
    if (last_tx != (exp)) { \
      printf("line %d: Expected reply 0x%x, got 0x%x.\n", \
	     __LINE__, exp, last_tx); \
    } } while(0)

#define RX_EXPECT(exp) do { \
    last_tx = -1; \
    state_machine_handle_tx_ready(); \
    if (last_tx != (exp)) { \
      printf("line %d: Expected rx 0x%x, got 0x%x.\n", \
	     __LINE__, exp, last_tx); \
    } } while(0)

void test_single_id(void) {
  struct state_shadow st;
  printf("%s start\n", __func__);
  init_state(&st, 0xdecafbad);
  restore_state(&st);
  EXPECT_ECHO(CMD_ID);
  TX_EXPECT(CMD_EOF, CMD_ID);

  RX_EXPECT(0xde);
  RX_EXPECT(0xca);
  RX_EXPECT(0xfb);
  RX_EXPECT(0xad);
  RX_EXPECT(CMD_EOF);
  EXPECT(IDLE, state);
  printf("%s end\n", __func__);
}

void test_second_id(void) {
  struct state_shadow st;
  printf("%s start\n", __func__);

  init_state(&st, 0xdecafbad);
  restore_state(&st);
  EXPECT_ECHO(CMD_ID);
  EXPECT_ECHO(CMD_ID);
  EXPECT_ECHO(1);
  EXPECT_ECHO(2);
  EXPECT_ECHO(3);
  EXPECT_ECHO(4);
  TX_EXPECT(CMD_EOF, CMD_ID);

  RX_EXPECT(0xde);
  RX_EXPECT(0xca);
  RX_EXPECT(0xfb);
  RX_EXPECT(0xad);
  RX_EXPECT(CMD_EOF);

  EXPECT(IDLE, state);
  printf("%s end\n", __func__);
}

static void test_single_pwm(void) {
  struct state_shadow st;
  printf("%s start\n", __func__);

  init_state(&st, 0xcafebabe);
  restore_state(&st);
  EXPECT_ECHO(CMD_PWM);
  TX_EXPECT(CMD_PWM, -1);
  TX_EXPECT(1, -1);
  TX_EXPECT(2, -1);
  TX_EXPECT(3, -1);
  TX_EXPECT(4, -1);
  TX_EXPECT(5, -1);
  TX_EXPECT(6, -1);

  EXPECT_ECHO(CMD_EOF);

  EXPECT(0x0102, pwm0);
  EXPECT(0x0304, pwm1);
  EXPECT(0x0506, pwm2);
    
  EXPECT(IDLE, state);
  printf("%s end\n", __func__);
}

static void test_second_pwm(void) {
  struct state_shadow st;
  printf("%s start\n", __func__);

  init_state(&st, 0xcafebabe);
  restore_state(&st);
  EXPECT_ECHO(CMD_PWM);
  TX_EXPECT(CMD_PWM, -1);
  TX_EXPECT(1, -1);
  TX_EXPECT(2, -1);
  TX_EXPECT(3, -1);
  TX_EXPECT(4, -1);
  TX_EXPECT(5, -1);
  TX_EXPECT(6, -1);

  EXPECT_ECHO(CMD_PWM);
  EXPECT_ECHO(7);
  EXPECT_ECHO(8);
  EXPECT_ECHO(9);
  EXPECT_ECHO(10);
  EXPECT_ECHO(11);
  EXPECT_ECHO(12);
  
  EXPECT_ECHO(CMD_EOF);

  EXPECT(0x0102, pwm0);
  EXPECT(0x0304, pwm1);
  EXPECT(0x0506, pwm2);
    
  EXPECT(IDLE, state);
  printf("%s end\n", __func__);
}

static int write_node(struct state_shadow *state, uint8_t byte) {
  restore_state(state);
  last_tx = -1;
  state_machine_handle_rx_byte(byte);
  save_state(state);
  return last_tx;
}

static int read_node(struct state_shadow *state) {
  restore_state(state);
  last_tx = -1;
  state_machine_handle_tx_ready();
  save_state(state);
  return last_tx;
}

static void write_chain(struct state_shadow *states, unsigned state_count, const uint8_t *indata, unsigned indata_count, uint8_t *outdata, unsigned *outdata_count) {
  unsigned i;
  *outdata_count = 0;
  for (i = 0; i < indata_count; i++) {
    int rx = write_node(&states[0], indata[i]);
    if (rx != -1) {
      outdata[*outdata_count] = rx;
      *outdata_count += 1;
    }
  }
  while (1) {
    int rx = read_node(&states[0]);
    if (rx == -1)
      break;
    outdata[*outdata_count] = rx;
    *outdata_count += 1;
  }

#if 0
  for (i = 0; i < *outdata_count; i++) {
    printf("0/out[%d] = 0x%x\n", i, outdata[i]);
  }
#endif
  
  for (i = 1; i < state_count; i++) {
    unsigned j;
    unsigned count_in = *outdata_count;
    *outdata_count = 0;
    for (j = 0; j < count_in; j++) {
      int rx = write_node(&states[i], outdata[j]);
      if (rx != -1) {
	outdata[*outdata_count] = rx;
	*outdata_count += 1;
      }
    }
    while (1) {
      int rx = read_node(&states[i]);
      if (rx == -1)
	break;
      outdata[*outdata_count] = rx;
      *outdata_count += 1;
    }
#if 0
    for (j = 0; j < *outdata_count; j++) {
      printf("%d/out[%d] = 0x%x\n", i, j, outdata[j]);
    }
#endif
  }
}

static void test_chain_id(unsigned count) {
  assert(count > 0);
  struct state_shadow *states = malloc(count * sizeof(struct state_shadow));
  printf("%s start\n", __func__);
  unsigned i;
  for (i = 0; i < count; i++)
    init_state(&states[i], rand());

  uint8_t *outdata = malloc(20 + count * 5) ;
  unsigned outdata_count;
  {
    const uint8_t indata[] = {CMD_ID, CMD_EOF};
    write_chain(states, count, indata, sizeof(indata), outdata, &outdata_count);
  }
  EXPECT(CMD_ID, outdata[0]);
  for (i = 0; i < count; i++) {
    EXPECT(CMD_ID, outdata[1+i*5]);
    uint32_t id = ((uint32_t)outdata[2+i*5] << 24)
      | ((uint32_t)outdata[3+i*5] << 16)
      | ((uint32_t)outdata[4+i*5] << 8)
      | ((uint32_t)outdata[5+i*5] << 0);
    EXPECT(states[i].id, id);
  }
  EXPECT(CMD_EOF, outdata[outdata_count - 1]);
  
  printf("%s end\n", __func__);
  free(outdata);
  free(states);
}

static void test_chain_pwm(unsigned count) {
  assert(count > 0);
  struct state_shadow *states = malloc(count * sizeof(struct state_shadow));
  printf("%s start\n", __func__);
  unsigned i;
  for (i = 0; i < count; i++)
    init_state(&states[i], rand());

  uint8_t *outdata = malloc(20 + count * 7) ;
  unsigned outdata_count;
  uint16_t *pwms = malloc(count*3*sizeof(uint16_t));
  for (i = 0; i < count; i++) {
    pwms[0 + i * 3] = rand();
    pwms[1 + i * 3] = rand();
    pwms[2 + i * 3] = rand();
  }
  {
    uint8_t *indata = malloc(20 + count * 7);
    indata[0] = CMD_PWM;
    for (i = 0; i < count; i++) {
      indata[1 + i*7] = CMD_PWM;
      indata[2 + i*7] = pwms[0 + i * 3] >> 8;
      indata[3 + i*7] = pwms[0 + i * 3];
      indata[4 + i*7] = pwms[1 + i * 3] >> 8;
      indata[5 + i*7] = pwms[1 + i * 3];
      indata[6 + i*7] = pwms[2 + i * 3] >> 8;
      indata[7 + i*7] = pwms[2 + i * 3];
    }
    indata[1 + count*7] = CMD_EOF;
    
    write_chain(states, count, indata, 2+count*7, outdata, &outdata_count);
    free(indata);
  }
  EXPECT(CMD_PWM, outdata[0]);
  EXPECT(CMD_EOF, outdata[1]);
  for (i = 0; i < count; i++) {
    EXPECT(pwms[0 + i * 3], states[i].pwm[0]);
    EXPECT(pwms[1 + i * 3], states[i].pwm[1]);
    EXPECT(pwms[2 + i * 3], states[i].pwm[2]);
  }
  
  printf("%s end\n", __func__);
  free(pwms);
  free(outdata);
  free(states);
}

static void test_single_sync(void) {
  struct state_shadow st;
  printf("%s start\n", __func__);
  init_state(&st, 0xdecafbad);
  st.pwm[0] = rand();
  st.pwm[1] = rand();
  st.pwm[2] = rand();
  last_pwm[0] = -1;
  last_pwm[1] = -1;
  last_pwm[2] = -1;
  restore_state(&st);
  next_gpio = rand();
  adc_started = false;
  EXPECT_ECHO(CMD_SYNC);
  uint8_t x = rand();
  TX_EXPECT(x, x - 1);
  
  EXPECT(st.gpio_set, last_gpio);
  EXPECT(st.pwm[0], last_pwm[0]);
  EXPECT(st.pwm[1], last_pwm[1]);
  EXPECT(st.pwm[2], last_pwm[2]);
  save_state(&st);
  EXPECT(next_gpio, st.gpio_get);
  EXPECT(true, adc_started);
  EXPECT(IDLE, st.state);
  printf("%s end\n", __func__);
}
static void test_single_adc(void) {
  struct state_shadow st;
  printf("%s start\n", __func__);
  init_state(&st, 0xdecafbad);
  restore_state(&st);
  next_adc = rand();
  EXPECT_ECHO(CMD_ADC);
  TX_EXPECT(CMD_EOF, CMD_ADC);

  RX_EXPECT(next_adc >> 8);
  RX_EXPECT(next_adc & 0xff);
  RX_EXPECT(CMD_EOF);
  EXPECT(IDLE, state);
  printf("%s end\n", __func__);
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  test_single_id();
  test_second_id();

  test_single_pwm();
  test_second_pwm();

  test_single_sync();
  //test_second_sync()

  test_single_adc();
  //test_second_adc();

  //test_single_get_gpio();
  //test_sencond_get_gpio();
  
  //test_single_set_gpio();
  //test_second_get_gpio();

  //test_single_full();
  
  test_chain_id(1);
  test_chain_id(2);
  test_chain_id(100);

  test_chain_pwm(1);
  test_chain_pwm(2);
  test_chain_pwm(100);

  //test_chain_full(1);
  //test_chain_full(2);
  //test_chain_full(100);
  return 0;
}

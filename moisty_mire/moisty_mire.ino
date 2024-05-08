#define MEAN_BUFF_SIZE 100

const int sens_1_pin = 1;
const int sens_2_pin = 8;

void configure_adc(void);
void getNewVal(int);

typedef struct {
  int latestValue = 0;
  int meanBuffer[MEAN_BUFF_SIZE];
  int bufferIdx = 0;
  double mean = 0.0;
  bool hasWrapped = false;
} sensor_t;

sensor_t sensor1; // mean room temp = ~2627, water = ~1225
sensor_t sensor2; // mean room temp = 2653, water = 1200

void setup() {
  Serial.begin(9600);
}

void loop() {
  sensor1.latestValue = analogRead(sens_1_pin);
  sensor2.latestValue = analogRead(sens_2_pin);

  getNewVal(&sensor1);
  getNewVal(&sensor2);
  
  if (sensor1.hasWrapped && sensor2.hasWrapped)
    Serial.printf("Sensor1: %lf Sensor2: %lf\n", sensor1.mean, sensor2.mean);
  delay(10);
}

void configure_adc(void) {
  // TODO
}

void getNewVal(sensor_t *sens) {
  double sum = sens->mean * MEAN_BUFF_SIZE;
  sum -= sens->meanBuffer[sens->bufferIdx];
  sum += sens->latestValue;

  sens->mean = sum / MEAN_BUFF_SIZE;

  sens->meanBuffer[sens->bufferIdx] = sens->latestValue;

  ++sens->bufferIdx;

  if (sens->bufferIdx > 99) {
    sens->bufferIdx = 0;
    sens->hasWrapped = true;
  }
  
}


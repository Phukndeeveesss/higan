class pInputDI;

class InputDI : public Input {
public:
  bool cap(Setting);
  uintptr_t get(Setting);
  bool set(Setting, uintptr_t);

  bool key_down(uint16_t key);

  void clear();
  void poll();
  bool init();
  void term();

  InputDI();
  ~InputDI();

private:
  pInputDI &p;
};